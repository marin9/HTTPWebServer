#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mft.h"
#include "net.h"
#include "file.h"

#define HELP	0
#define STATUS	1
#define SERVER	2
#define PORT	3
#define PUT		4
#define GET		5
#define QUIT	6

#define BUFFLEN		(HEADLEN+DATALEN)

int GetCommand(char *buff);
void PutFile(unsigned short port, char *host, char *name);
void GetFile(unsigned short port, char *host, char *name);
//TODO add request retransmission 

int main(){
	unsigned short port=1900;
	char addr[INET_ADDRSTRLEN]={0};
	char hostname[64]={0};
	char buffer[BUFFLEN];
	
	while(1){
		printf("\n>");
		
		memset(buffer, 0, BUFFLEN);
		scanf("%s", buffer);
		int cm=GetCommand(buffer);
		
		if(cm==HELP){
			printf("\nstatus - print current status(server and port)\n");
			printf("server:hostaddr - set host server address\n");
			printf("port:portnumber - set server port\n");
			printf("put:filename - upload file to server\n");
			printf("get:filename - download file from server\n");
			printf("quit - exit\n");
			
		}else if(cm==STATUS){
			printf("Server: %s (%s)\n", hostname, addr);
			printf("Port: %d\n", port);
			
		}else if(cm==SERVER){
			memset(addr, 0, INET_ADDRSTRLEN);
			strcpy(hostname, buffer);
			
			GetIpFromName(buffer, addr);
			GetNameFromIP(buffer, hostname);
			
		}else if(cm==PORT) port=atoi(buffer);			
		else if(cm==PUT) PutFile(port, addr, buffer);		
		else if(cm==GET) GetFile(port, addr, buffer);		
		else if(cm==QUIT) break;			
		else printf("Illegal command. Type 'help'.\n");
	}	
	return 0;
}

int GetCommand(char *buff){
	int i, j, code;
	char command[7];
	
	for(i=0;i<7 && buff[i]!=':';++i){
		command[i]=buff[i];
	}
	command[i]=0;
	
	if(strcmp(command, "HELP")==0 || strcmp(command, "help")==0) code=HELP;
	else if(strcmp(command, "STATUS")==0 || strcmp(command, "status")==0) code=STATUS;
	else if(strcmp(command, "SERVER")==0 || strcmp(command, "server")==0) code=SERVER;
	else if(strcmp(command, "PORT")==0 || strcmp(command, "port")==0) code=PORT;
	else if(strcmp(command, "PUT")==0 || strcmp(command, "put")==0) code=PUT;
	else if(strcmp(command, "GET")==0 || strcmp(command, "get")==0) code=GET;
	else if(strcmp(command, "QUIT")==0 || strcmp(command, "quit")==0) code=QUIT;
	else return -1;
	
	++i;
	for(j=0;j<(int)(BUFFLEN-i) && buff[i]!='\0';++j, ++i){
		buff[j]=buff[i];	
	}
	buff[j]=0;
	return code;
}    

void PutFile(unsigned short port, char *host, char *name){
	int sock=SocketUDP(0);
	SetSocketTimeout(sock, RETTIMEO);
	
	struct sockaddr_in addr;
	struct sockaddr_in saddr;
	socklen_t len=sizeof(addr);
	socklen_t slen=sizeof(saddr);
	struct packet buff;
	
	memset(&addr, 0, sizeof(addr));   
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	inet_pton(AF_INET, host, &(addr.sin_addr));  
	
	FILE *file=fopen(name, "rb");
	if(file==NULL){
		printf("\x1B[31mERROR:\x1B[0m Open file fail: %s.\n", strerror(errno));
		close(sock);
		return;
	}
	
	buff.code=WRITE;
	strcpy((char*)&buff+HEADLEN, name);
	SendTo(sock, (char*)&buff, BUFFLEN, &addr);
	
	int i;
	for(i=0;i<RETRNUM;++i){
		if(RecvFrom(sock, (char*)&buff, BUFFLEN, &addr, &len)==-1) continue;	
		if(buff.code==ACK && buff.num==0){
			memcpy(&saddr, &addr, len);
			slen=len;
			break;
			
		}else if(buff.code==ERROR){
			printf("\x1B[33m%s\x1B[0m \n", buff.data);
			fclose(file);
			close(sock);
			return;
		}
	}
	if(i==RETRNUM){
		printf("\x1B[33mTimeout\x1B[0m \n");
		fclose(file);
		close(sock);
		return;
	}
	
	int n, packNum=1;
	struct packet recvBuff;
		
	while(!feof(file)){
		n=fread((char*)&buff+HEADLEN, 1, DATALEN, file);
		if(n<0){
			printf("\x1B[31mError:\x1B[0m Read file error: %s.\n", strerror(errno));
			fclose(file);
			close(sock);
			return;
		}
		
		buff.code=DATA;
		buff.num=packNum;		

		for(i=0;i<RETRNUM;++i){
			SendTo(sock, (char*)&buff, n+HEADLEN, &addr);
			
			int s=RecvFrom(sock, (char*)&recvBuff, HEADLEN, &saddr, &slen);		
			if(s!=-1 && recvBuff.code==ACK && recvBuff.num==packNum && equalsAddr(&addr, &saddr)){
				break;
				
			}else if(recvBuff.code==ERROR){
				printf("\x1B[33m%s\x1B[0m \n", recvBuff.data);
				fclose(file);
				close(sock);
				return;
			}
		}
		if(i==RETRNUM){
			printf("\x1B[33mTimeout.\x1B[0m \n");
			fclose(file);
			close(sock);
			return;
		}
		++packNum;
		
		if(packNum%1000==0){
			char str[16];
			sizeToH(DATALEN*packNum, str, 16);
			printf("%s received\n", str);
		}
	}
	
	printf("\x1B[32mFinish.\x1B[0m \n");
	fclose(file);
	close(sock);
}

void GetFile(unsigned short port, char *host, char *name){
	int sock=SocketUDP(0);
	SetSocketTimeout(sock, RETTIMEO);
	
	struct sockaddr_in addr;
	struct sockaddr_in saddr;
	socklen_t len=sizeof(addr);
	socklen_t slen=sizeof(saddr);
	struct packet buff;
	
	memset(&addr, 0, sizeof(addr));   
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	inet_pton(AF_INET, host, &(addr.sin_addr));  
	
	FILE *file=fopen(name, "w+b");
	if(file==NULL){
		printf("\x1B[31mERROR:\x1B[0m Create file fail: %s.\n", strerror(errno));
		close(sock);
		return;
	}
	
	buff.code=READ;
	strcpy((char*)&buff+HEADLEN, name);
	
	SendTo(sock, (char*)&buff, BUFFLEN, &addr);
	
	int i, n, packNum=1;
	while(1){
		for(i=0;i<RETRNUM;++i){
			if(packNum==1){			
				n=RecvFrom(sock, (char*)&buff, BUFFLEN, &saddr, &slen);
				memcpy(&addr, &saddr, slen);
				len=slen;
			}
			else n=RecvFrom(sock, (char*)&buff, BUFFLEN, &addr, &len);
	
			if(n==-1) continue;
			else if(packNum!=1 && !equalsAddr(&addr, &saddr)) continue;
			else if(buff.code==DATA && buff.num==packNum) break;
			else if(buff.code==DATA && buff.num<packNum){
				SendAck(sock, (char*)&buff, &addr, buff.num);
				continue;
			}
			else if(buff.code==ERROR){
				printf("\x1B[33m%s\x1B[0m", buff.data);
				remove(name); 
				fclose(file);
				close(sock);
				return;
			}
		}		
		if(i==RETRNUM){
			printf("\x1B[33mTimeout.\x1B[0m \n");
			remove(name);
			break;
		}		
		
		if(fwrite((char*)&buff+HEADLEN, 1, (n-HEADLEN), file)!=(n-HEADLEN)){
			printf("\x1B[31mERROR:\x1B[0m File data write fail: %s.\n", strerror(errno));
			remove(name);
			break;
		}
		
		SendAck(sock, (char*)&buff, &addr, packNum);
		
		if(n<DATALEN){
			printf("\x1B[32mFinish.\x1B[0m \n");
			break;
		}
		++packNum;

		if(packNum%1000==0){
			char str[16];
			sizeToH(DATALEN*packNum, str, 16);
			printf("%s received\n", str);
		}
	}	
	fclose(file);
	close(sock);
}

