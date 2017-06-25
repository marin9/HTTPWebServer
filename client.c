#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mft.h"

#define HELP	0
#define STATUS	1
#define SERVER	2
#define PORT	3
#define PUT		4
#define GET		5
#define QUIT	6

#define BUFFLEN		(HEADLEN+DATALEN)


int GetCommand(char *buff);

//TODO  client.c
int main(){
	unsigned short port=1900;
	char host[INET_ADDRSTRLEN];
	
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
			
		}else if(0){
			//TODO
			
		}else{
			printf("Illegal command. Type 'help'.\n");
		}
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


