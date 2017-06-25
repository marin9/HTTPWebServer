#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "net.h"
#include "file.h"
#include "web.h"
#include "mft.h"

#define BUFFLEN 512


void GetOpt(int argc, char **argv, char *dir, unsigned short *uport, unsigned short *tport, int *write);
void LoadHtmlDoc();
void printServerInfo(int uport, int tport, char *dir);
void sigchld_handler();

char *HTMLDOC=NULL;


int main(int argc, char **argv){
	unsigned short tport=80;
	unsigned short uport=1900;
	char workingDir[NAMELEN];
	int write;
		
	GetOpt(argc, argv, workingDir, &uport, &tport, &write);
	LoadHtmlDoc();
		
	int ssock=SocketTCP(tport);

	struct sockaddr_in caddr;
	socklen_t clen=sizeof(caddr);
	int csock;
	
	struct sigaction sa;
	sa.sa_handler=sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=SA_RESTART;
	
	if(sigaction(SIGCHLD, &sa, NULL)==-1){
		printf("\x1B[33mERROR:\x1B[0m Sigaction error.\n");
		exit(1);
	}
	
	StartServer(uport, workingDir, write);
	
	while(1){
		if((csock=accept(ssock, (struct sockaddr*)&caddr, &clen))==1){
			printf("\x1B[33mERROR:\x1B[0m TCP client accept fail: %s\n", strerror(errno));
			close(csock);
			continue;
		}
		
		pid_t p=fork();
		if(p==-1){
			printf("\x1B[33mERROR:\x1B[0m Create tcp listen process fail.\n");
			exit(3);
		}else if(p==0){
			close(ssock);
						
		}else{
			close(csock);
			continue;
		}
		
		web(csock, workingDir);
		close(csock);
		exit(0);
	}
	
	return 0;
}



void GetOpt(int argc, char **argv, char *dir, unsigned short *uport, unsigned short *tport, int *write){
	int c;
	int sum=0;
	
	dir[0]=0;
	*write=0;

	while((c=getopt(argc, argv, "u:d:p:w"))!=-1){
    	switch(c){
     		case 'd':			
				strncpy(dir, optarg, NAMELEN);
				sum+=2;
        		break;
      		case 'p':
       			*tport=(unsigned short)atoi(optarg);
				sum+=2;
       			break;
			case 'u':
				*uport=(unsigned short)atoi(optarg);
				sum+=2;
				break;
			case 'w':
				*write=1;
				++sum;
      		default:
        		printf("Usage: [-u control_udp_port] [-d directory] [-p tcp_port]\n");
				exit(1);
      	}
	}
	
	if(argc!=(sum+1)){
		printf("Usage: [-u control_udp_port] [-d directory] [-p tcp_port]\n");
		exit(1);
	}
	
	if(dir[0]!=0 && !dirExist(dir)){
		printf("\x1B[31mERROR:\x1B[0m Directory not exist.\n");
		exit(1);
	}

	if(dir[0]==0) getcwd(dir, NAMELEN);

	printServerInfo(*uport, *tport, dir);
}

void LoadHtmlDoc(){
	int len=fileSize("res/doc.html");
	FILE *f=fopen("res/doc.html", "rb");
	
	if(len==-1 || f==NULL){
		printf("\x1B[31mERROR:\x1B[0m No html doc.\n");
		exit(1);
	}
	
	HTMLDOC=(char*)malloc(len+1);

	int i=0;
	int n=0;
	while((n=fread(HTMLDOC+i, 1, BUFFLEN, f))>0){
		i=i+n;
	}
	fclose(f);
}

void printServerInfo(int uport, int tport, char *dir){
	printLocalAddrs();
	printf("Control port:  %d\n", uport);
	printf("Web port:      %d\n", tport);
	printf("\nWorking directory: %s\n", dir);
}

void sigchld_handler(){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
