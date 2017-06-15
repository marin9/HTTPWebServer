#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "net.h"
#include "file.h"
#include "web.h"

#define MAXPATHLEN 128
#define BUFFLEN 512


void GetOpt(int argc, char **argv, char *dir, unsigned short *uport, unsigned short *tport);
void LoadHtmlDoc();
void printServerInfo(int uport, int tport, char *dir);
void startServer(unsigned short port, char* dir);
void sigchld_handler();

char *HTMLDOC=NULL;


int main(int argc, char **argv){
	unsigned short tport=80;
	unsigned short cuport=999;
	
	pid_t  procTcp;
	char serverOn=0;
	char workingDir[MAXPATHLEN];
		
	GetOpt(argc, argv, workingDir, &cuport, &tport);
	LoadHtmlDoc();

	struct sockaddr_in addr;
	socklen_t alen=sizeof(addr);
	char buffer[BUFFLEN];
	int usock=SocketUDP(cuport);
		
	struct sigaction sa;
	sa.sa_handler=sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=SA_RESTART;
	
	if(sigaction(SIGCHLD, &sa, NULL)==-1){
		printf("\x1B[33mERROR:\x1B[0m Sigaction error.\n");
		exit(1);
	}

	while(1){
		int s=recvfrom(usock, buffer, BUFFLEN, 0, (struct sockaddr*)&addr, &alen);

		if(s<0){
			printf("\x1B[33mERROR:\x1B[0m Control udp recv fail: %s\n", strerror(errno));
			sleep(1);
			continue;
		}

		if(strcmp(buffer, "HELLO")==0){
			gethostname(buffer, BUFFLEN);
			sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);			
			
		}else if(strcmp(buffer, "QUIT")==0){
			strcpy(buffer, "shutdown ok.\n");
			sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
			
			close(usock);
			if(serverOn) kill(procTcp, SIGKILL);
			printf("Server shutdown.\n");
			break;

		}else if(strcmp(buffer, "ON")==0){
			if(serverOn){
				strcpy(buffer, "\n");
				sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
				continue;
			}

			procTcp=fork();
			if(procTcp==-1){
				printf("\x1B[33mERROR:\x1B[0m Create tcp listen process fail.\n");
				exit(3);
			}else if(procTcp==0){
				printf("\x1B[32mServer started.\x1B[0m\n");
				strcpy(buffer, "on\n");
				sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
				
				startServer(tport, workingDir);
			}else{
				serverOn=1;
			}

		}else if(strcmp(buffer, "OFF")==0){
			if(!serverOn){
				strcpy(buffer, "\n");
				sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
				
				continue;
			}
			strcpy(buffer, "off\n");
			sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
		
			serverOn=0;
			printf("\x1B[34mServer stopped.\x1B[0m\n");
			kill(procTcp, SIGKILL);
			
		}else if(strcmp(buffer, "HELP")==0){
			strcpy(buffer, "\nON - server listen turn on.\nOFF - server listen turn off.\nQUIT - shutdown server.\n");
			sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);
		
		}else{
			strcpy(buffer, "unknown command.\n");
			sendto(usock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, alen);			
		}
	}	
	return 0;
}

void startServer(unsigned short port, char* dir){
	int ssock=SocketTCP(port);

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
		
		web(csock, dir);
		close(csock);
		exit(0);
	}
}

void GetOpt(int argc, char **argv, char *dir, unsigned short *uport, unsigned short *tport){
	int c;
	int sum=0;
	
	dir[0]=0;

	while((c=getopt(argc, argv, "u:d:p:"))!=-1){
    	switch(c){
     		case 'd':			
				strncpy(dir, optarg, MAXPATHLEN);
				++sum;
        		break;
      		case 'p':
       			*tport=(unsigned short)atoi(optarg);
				++sum;
       			break;
			case 'u':
				*uport=(unsigned short)atoi(optarg);
				++sum;
				break;
      		default:
        		printf("Usage: [-u control_udp_port] [-d directory] [-p tcp_port]\n");
				exit(1);
      	}
	}
	
	if(argc!=(2*sum+1)){
		printf("Usage: [-u control_udp_port] [-d directory] [-p tcp_port]\n");
		exit(1);
	}
	
	if(dir[0]!=0 && !dirExist(dir)){
		printf("\x1B[31mERROR:\x1B[0m Directory not exist.\n");
		exit(1);
	}

	if(dir[0]==0) getcwd(dir, MAXPATHLEN);

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
	while((n=fread(HTMLDOC+i, 1, 512, f))>0){
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
