#include "mft.h"
#include "net.h"
#include <unistd.h>

void StartServer(unsigned short port, char* dir, int write){	
	if(CreateProcess()) return;
	
	int sock=SocketUDP(port);
	struct sockaddr_in addr;
	socklen_t len=sizeof(addr);
	struct packet buffer;
	
	while(1){
		RecvFrom(sock, (char*)&buffer, sizeof(buffer), &addr, &len);	
		if(CreateProcess()) continue;
		else close(sock);
		
		int csock=SocketUDP(0);
		SetSocketTimeout(sock, RETTIMEO);
	
		if(buffer.code==READ) ReadFile(csock, (char*)&buffer, &addr, dir);
		else if(buffer.code==WRITE && write) WriteFile(csock, (char*)&buffer, &addr, dir);
		else if(buffer.code==WRITE && !write) MSendError(csock, (char*)&buffer, &addr, ACCESS_VIOLATION, "Access violation.\n");
		else MSendError(csock, (char*)&buffer, &addr, ILLEGAL_OPERATION, "Unsupported request.\n");
		
		close(csock);
		break;
	}
	close(sock);
	exit(0);
}

void SetSocketTimeout(int sock, int sec){
	struct timeval timeout;      
	timeout.tv_sec=sec;
	timeout.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))<0){
		printf("\x1B[33mERROR:\x1B[0m Set UDP socket option timeout fail: %s.\n", strerror(errno));
		exit(4);
	}
}

int CreateProcess(){
	int s=fork();
	if(s==-1){
		printf("\x1B[33mERROR:\x1B[0m Create UDP process fail: %s.\n", strerror(errno));
		exit(1);
	}
	return s;
}

int RecvFrom(int sock, char* buff, int size, struct sockaddr_in* addr, socklen_t *len){
	int n=recvfrom(sock, buff, size, 0, (struct sockaddr*)addr, len);
	if(n<0){
		if(errno==EAGAIN || errno==EWOULDBLOCK) return -1;
		printf("\x1B[33mERROR:\x1B[0m Receive UDP request fail: %s.\n", strerror(errno));
		exit(2);
	}
	return n;
}

void SendTo(int sock, char* buff, int size, struct sockaddr_in* addr){
	int n=sendto(sock, buff, size, 0, (struct sockaddr*)addr, sizeof(*addr));
	if(n!=size){
		printf("\x1B[33mERROR:\x1B[0m Send UDP data fail: %s.\n", strerror(errno));
		exit(3);
	}
}

void MSendError(int csock, char *buff, struct sockaddr_in *addr, int errcode, char* msg){
	int ilen=sizeof(int);
	
	((struct packet*)buff)->code=ERROR;
	((struct packet*)(buff+ilen))->num=errcode;
	strcpy(buff+2*ilen, msg);
	
	int pack_len=2*ilen+strlen(buff+2*ilen)+1;
	SendTo(csock, buff, pack_len, addr);
}

void ReadFile(int sock, char *buff, struct sockaddr_in* addr, char *dir){
	if((strlen(buff+HEADLEN)+strlen(dir))>DATALEN){
		printf("\x1B[33mERROR:\x1B[0m Path too long.\n");
		return;
	}
	
	char name[DATALEN];
	strcpy(name, dir);
	strcat(name, "/");
	strcat(name, buff+HEADLEN);
	
	FILE *file=fopen(name, "rb");
	if(file==NULL){
		if(errno==ENOENT) MSendError(sock, buff, addr, FILE_NOT_FOUND, "File not found.\n");
		else if(errno==EACCES) MSendError(sock, buff, addr, ACCESS_VIOLATION, "Access violation.");
		else MSendError(sock, buff, addr, NOT_DEFINED, strerror(errno));
		return;
	}
	
	int i, packNum=1;
	struct sockaddr_in rcvaddr;
	socklen_t rcvlen=sizeof(rcvaddr);
	int ackbuff[2];
		
	while(!feof(file)){
		int n=fread(buff+HEADLEN, 1, DATALEN, file);
		if(n<0){
			MSendError(sock, buff, &rcvaddr, NOT_DEFINED, strerror(errno));
			break;
		}
		
		((struct packet*)buff)->code=DATA;
		((struct packet*)buff)->num=packNum;				
		for(i=0;i<RETRNUM;++i){
			SendTo(sock, buff, n+HEADLEN, addr);
						
			if(RecvFrom(sock, (char*)ackbuff, HEADLEN, &rcvaddr, &rcvlen)==-1) continue;
			else if(ackbuff[0]==ACK && ackbuff[1]==packNum && equalsAddr(addr, &rcvaddr)) break;
		}
		
		if(i==RETRNUM) break;
		++packNum;
	}	
	fclose(file);
}

void WriteFile(int sock, char *buff, struct sockaddr_in* addr, char *dir){
	if((strlen(buff+HEADLEN)+strlen(dir))>DATALEN){
		printf("\x1B[33mERROR:\x1B[0m Path too long.\n");
		return;
	}
	
	char name[DATALEN];
	strcpy(name, dir);
	strcat(name, "/");
	strcat(name, buff+HEADLEN);
	
	if(TestFileExist(name)){
		MSendError(sock, buff, addr, FILE_ALREADY_EXIST, "File already exist.\n");
		return;
	}
	
	FILE *file=fopen(name, "w+b");
	if(file==NULL){
		if(errno==EACCES) MSendError(sock, buff, addr, ACCESS_VIOLATION, "Access violation.");
		else MSendError(sock, buff, addr, NOT_DEFINED, strerror(errno));
		return;
	}
	
	int i, n;
	int packNum=0;
	struct sockaddr_in rcvaddr;
	socklen_t rcvlen=sizeof(rcvaddr);
	
	SendAck(sock, buff, addr, packNum);
	while(1){
		++packNum;		

		for(i=0;i<RETRNUM;++i){
			n=RecvFrom(sock, buff, sizeof(struct packet), &rcvaddr, &rcvlen);
			if(n==-1) continue;
	
			if(!equalsAddr(addr, &rcvaddr)) continue;
			else if( ((struct packet*)buff)->code==DATA && ((struct packet*)buff)->num==packNum){
				int s=fwrite(buff+HEADLEN, 1, n-HEADLEN, file); 
				if(s!=(int)(n-HEADLEN)){
					MSendError(sock, buff, addr, NOT_DEFINED, strerror(errno));
					fclose(file);
					remove(name);
					return;
				}
				break;
			}else if( ((struct packet*)buff)->code==DATA && ((struct packet*)buff)->num<packNum){
				SendAck(sock, buff, addr, ((struct packet*)buff)->num);			
			}
		}
		if(i==RETRNUM){
			remove(name);
			break;
		}
	
		SendAck(sock, buff, addr, packNum);
		if(n<DATALEN) break;	
	}	
	fclose(file);	
}

void SendAck(int sock, char *buff, struct sockaddr_in* addr, int num){
	((struct packet*)buff)->code=ACK;
	((struct packet*)buff)->num=num;
	
	SendTo(sock, buff, HEADLEN, addr);
}

int TestFileExist(char *path){
	int s=access(path, F_OK);
	
	if(!s) return 1;		
	else return 0;	
}
