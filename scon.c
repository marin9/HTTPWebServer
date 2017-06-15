#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "net.h"

#define BUFFLEN		256


void GetOpt(int argc, char **argv, unsigned short *port);
void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port);
void Send(int sock, char* buff, struct sockaddr_in *addr);
void Recv(int sock, char* buff);
void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr);
void SetSockTime(int sock);
void toup(char *buff);


int main(int argc, char **argv){	
	int sock;	
	char buffer[BUFFLEN];
	unsigned short port;
	struct sockaddr_in addr;

	GetOpt(argc, argv, &port);
	
	sock=SocketUDP(0);
	SetSockTime(sock);
	PrepareAddrBroadcast(sock, &addr, port);		
	GetServerAddr(sock, buffer, &addr); 

	while(1){
		printf(">");
		scanf("%s", buffer);
		
		toup(buffer);
		if(strcmp(buffer, "EXIT")==0) break;
		
		Send(sock, buffer, &addr);
		Recv(sock, buffer);
	}
	return 0;
}


void GetOpt(int argc, char **argv, unsigned short *port){
	*port=999;

	if(argc==3){
		if(argv[1][0]=='-' && argv[1][1]=='u' && strlen(argv[1])==2){
			*port=(unsigned short)atoi(argv[2]);
		}else{
			printf("Usage: [-u control_port]\n");
			exit(1);
		}
	}else if(argc>3 || argc==2){
		printf("Usage: [-u control_port]\n");
		exit(1);
	}
}

void PrepareAddrBroadcast(int socket, struct sockaddr_in *addr, unsigned short port){
	memset(addr, 0, sizeof(*addr));   
	addr->sin_family=AF_INET;
	addr->sin_port=htons(port);
	inet_pton(AF_INET, "255.255.255.255", &(addr->sin_addr));

	int on=1;
	if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0){
		printf("\x1B[31mERROR:\x1B[0m Set socket broadcast option fail.\n");
		exit(2);
	}
}

void Send(int sock, char* buff, struct sockaddr_in *addr){
	int msglen=strlen(buff);
	if(sendto(sock, buff, msglen+1, 0, (struct sockaddr*)addr, sizeof(*addr))!=(msglen+1)){
		printf("\x1B[31mERROR:\x1B[0m Send fail: %s\n", strerror(errno));
		exit(3);
	}
}

void Recv(int sock, char* buff){
	memset(buff, 0, BUFFLEN);   

	int ans=0;
	for(int i=0;i<3;++i){
		int s=recv(sock, buff, BUFFLEN, 0);

		buff[s]=0;
		if(s==-1){
			if(errno==EAGAIN || errno==EWOULDBLOCK) printf(".\n");
			else{
				printf("\x1B[31mERROR:\x1B[0m Receive fail: %s\n", strerror(errno));
				break;
			}
		}else{
			ans=1;
			break;
		}		
	}

	if(ans) printf("\x1B[34mServer:\x1B[0m %s\n", buff);
}

void SetSockTime(int sock){
	struct timeval tv;
	tv.tv_sec=3;  
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval))){
		printf("\x1B[31mERROR:\x1B[0m Set socket timeout option fail.\n");
		exit(2);
	}
}

void GetServerAddr(int sock, char *buff, struct sockaddr_in *addr){
	socklen_t addrlen=sizeof(*addr);

	strcpy(buff, "HELLO");
	
	while(1){
		Send(sock, buff, addr); 
		int s=recvfrom(sock, buff, BUFFLEN, 0, (struct sockaddr*)addr, &addrlen);

		buff[s]=0;
		if(s==-1){
			if(errno==EAGAIN || errno==EWOULDBLOCK) printf("\x1B[33mTime out...\x1B[0m \n");
			else{
				printf("\x1B[31mERROR:\x1B[0m Receive fail: %s\n", strerror(errno));
				break;
			}
		}else{
			break;
		}		
	}
	printf("\x1B[32mServer control ready.\x1B[0m \nHost name: %s\n", buff);
}

void toup(char *buff){
	int l=strlen(buff);
	for(int i=0;i<l;++i){
			if(buff[i]>='a' && buff[i]<='z') buff[i]=buff[i]-32;
	}
}

