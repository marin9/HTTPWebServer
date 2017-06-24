#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define HOSNAMETLEN		128

void printLocalAddrs();
int SocketUDP(unsigned short port);
int SocketTCP(unsigned short port);
int equalsAddr(struct sockaddr_in* addr1, struct sockaddr_in* addr2);

#endif

