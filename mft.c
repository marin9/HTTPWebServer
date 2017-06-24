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
		struct timeval timeout;      
		timeout.tv_sec=3;
		timeout.tv_usec=0;
		if(setsockopt(csock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))<0){
			printf("\x1B[33mERROR:\x1B[0m Set UDP socket option timeout fail: %s.\n", strerror(errno));
			exit(4);
		}
	
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
		if(errno==EAGAIN || errno==EWOULDBLOCK) return 0;
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
	
	int packNum=1;
	int i;
	struct sockaddr_in rcvaddr;
	socklen_t rcvlen=sizeof(rcvaddr);
	int ackResponse[2];
		
	while(!feof(file)){
		int n=fread(buff+HEADLEN, 1, DATALEN, file);
		((struct packet*)buff)->code=DATA;
		((struct packet*)buff)->num=packNum;		
		
		for(i=0;i<5;++i){
			SendTo(sock, buff, n+HEADLEN, addr);
			
			if(!RecvFrom(sock, (char*)ackResponse, HEADLEN, &rcvaddr, &rcvlen)){
				continue;
			
			}else if(!equalsAddr(addr, &rcvaddr)){
				char tmpbuff[32];
				MSendError(sock, tmpbuff, &rcvaddr, UNKNOWN_PORT, "Unknown port.\n");
				
			}else if(ackResponse[0]==ACK && ackResponse[1]==packNum){
				break;
			}
		}
		++packNum;
	}	
	fclose(file);
}

void WriteFile(int sock, char *buff, struct sockaddr_in* addr, char *dir){
	//TODO
	
}

