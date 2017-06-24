#include "net.h"


void printLocalAddrs(){
	char hostname[HOSNAMETLEN];
    gethostname(hostname, HOSNAMETLEN);
	printf("Host name: %s\n\n", hostname);

	struct ifaddrs *addr, *paddr;
    if(getifaddrs(&addr)==-1){
		printf("\x1B[33mWARNING:\x1B[0m Unknown local address.\n");
    }
	paddr=addr;
	
    char host[NI_MAXHOST];

	printf("Interfaces:\nAddress \t Netmask\n");
    for(;addr!=NULL;addr=addr->ifa_next){
        if(addr->ifa_addr->sa_family==AF_INET){
            getnameinfo(addr->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            printf("%s\t", host);
			getnameinfo(addr->ifa_netmask, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            printf("%s\n", host);
        }
    }
	printf("\n");
	freeifaddrs(paddr);
}

int SocketUDP(unsigned short port){
	int sock;
	struct sockaddr_in addr;

	if((sock=socket(PF_INET, SOCK_DGRAM, 0))==-1){
		printf("\x1B[31mERROR:\x1B[0m UDP socket bind fail: %s\n", strerror(errno));
		exit(2);
	}
	
	int on=1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))==-1){
		printf("\x1B[31mERROR:\x1B[0m UDP set socket opt fail. %s\n", strerror(errno));
		exit(2);
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0){
		printf("\x1B[31mERROR:\x1B[0m UDP socket bind fail: %s\n", strerror(errno));
		exit(2);
	}

	return sock;
}

int SocketTCP(unsigned short port){
	int sock;
	struct sockaddr_in addr;

	if((sock=socket(PF_INET, SOCK_STREAM, 0))==-1){
		printf("\x1B[31mERROR:\x1B[0m TCP socket bind fail: %s\n", strerror(errno));
		exit(2);
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;

	int on=1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))==-1){
		printf("\x1B[31mERROR:\x1B[0m TCP set socket opt fail. %s\n", strerror(errno));
		exit(2);
	}

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0){
		printf("\x1B[31mERROR:\x1B[0m TCP socket bind fail: %s\n", strerror(errno));
		exit(2);
	}

	if(listen(sock, 10)==-1){
		printf("\x1B[31mERROR:\x1B[0m TCP socket listen fail. %s\n", strerror(errno));
		exit(2);
	}

	return sock;
}




