#ifndef MFT_H
#define MFT_H
#include <sys/socket.h>
#include <netinet/in.h>

#define READ	1
#define WRITE	2
#define DATA	3
#define ACK		4
#define ERROR	5

#define NOT_DEFINED			0
#define FILE_NOT_FOUND		1
#define ACCESS_VIOLATION	2
#define DISK_FULL			3
#define ILLEGAL_OPERATION	4
#define UNKNOWN_PORT		5
#define FILE_ALREADY_EXIST	6

#define HEADLEN		(2*sizeof(int))
#define DATALEN		1024

struct packet{
	int code;
	int num;
	char data[DATALEN];
};

void StartServer(unsigned short port, char *dir, int write);
int CreateProcess();
int RecvFrom(int sock, char *buff, int size, struct sockaddr_in* addr, socklen_t *len);
void SendTo(int sock, char* buff, int size, struct sockaddr_in* addr);
void MSendError(int csock, char *buff, struct sockaddr_in *addr, int errcode, char* msg);
void ReadFile(int sock, char *buff, struct sockaddr_in* addr, char *dir);
void WriteFile(int sock, char *buff, struct sockaddr_in* addr, char *dir);

#endif
