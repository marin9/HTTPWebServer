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
		RecvFrom(sock, (char*)&buffer, &addr, &len);	
		if(CreateProcess()) continue;
		else close(sock);
		
		int csock=SocketUDP(0);
	
		if(buffer.code==READ) ;//TODO
		else if(buffer.code==WRITE) ;//TODO
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

int RecvFrom(int sock, char* buff, struct sockaddr_in* addr, socklen_t *len){
	int n=recvfrom(sock, buff, sizeof(struct packet), 0, (struct sockaddr*)addr, len);
	if(n<0){
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

