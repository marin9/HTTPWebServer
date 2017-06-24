#include <unistd.h>
#include "web.h"
#include "file.h"
#include "mimetype.h"

#define BUFFLEN  2048

extern char *HTMLDOC;


void web(int sock, char *dir){
	char buffer[BUFFLEN];
	char ip[INET_ADDRSTRLEN];

	RecvRequest(sock, buffer, BUFFLEN, ip, INET_ADDRSTRLEN);

	if(!GetRequest(buffer, BUFFLEN)){
		SendError(sock, 405);
		return;
	}

	if(strcmp(buffer, "/index.html")==0){
		buffer[0]='/';
		buffer[1]=0;
		SendList(sock, buffer, dir);

	}else if(strcmp(buffer, "/favicon.ico")==0){
		SendFile(sock, buffer+1);
		
	}else if(strncmp(buffer, "/res/", 4)==0){
		char path[NAMELEN];
		getcwd(path, NAMELEN);
		strncat(path, buffer, NAMELEN);		
		SendFile(sock, path);
		
	}else{
		char path[BUFFLEN];
		sprintf(path, "%s", dir); 
		strncat(path, buffer, BUFFLEN-strlen(path)-1);

		if(isDir(path)) SendList(sock, path, buffer);
		else SendFile(sock, path);
	}
}

void RecvRequest(int csock, char *buffer, int blen, char *c_ip, int ip_len){
	int sum=0;
	int ok=0;

	struct sockaddr_in caddr;
	socklen_t alen=sizeof(caddr);

	while(sum<blen && !ok){
		int n=recvfrom(csock, buffer+sum, blen-sum, 0, (struct sockaddr*)&caddr, &alen);

		if(n<1){
			close(csock);
			exit(11);
		}

		ok=isFinish(buffer, sum, sum+n);	
		sum=sum+n;
	}
	inet_ntop(AF_INET, &caddr.sin_addr, c_ip, ip_len);
}

int isFinish(char *buff, int sum, int n){
	for(int i=sum;i<n-3;++i){
		if(buff[i]=='\r' && buff[i+1]=='\n' && buff[i+2]=='\r' && buff[i+3]=='\n'){
			return 1;
		}
	}
	return 0;
}

int GetRequest(char *buff, int blen){
	if(!(buff[0]=='G' && buff[1]=='E' && buff[2]=='T')){
		return 0;
	}

	int i=3, j;
	while(buff[i]!=' ') ++i;
	++i;
	
	for(j=0;buff[i]!=' ' && i<blen; ++j, ++i) buff[j]=buff[i];
	buff[j]=0;

	return 1;
}

void SendError(int sock, int code){
	char content[128];
	char buff[BUFFLEN];
	
	if(code==404){
		sprintf(content, "<html><body><h1>%d: File or directory not exist.</h1></body></html>", code);
		sprintf(buff, "HTTP/1.1 %d\r\nContent-type: text/html\r\nContent-length: %ld\r\n\r\n%s", code, strlen(content), content);
		send(sock, buff, strlen(buff), 0);
		
	}else if(code==405){		
		sprintf(content, "<html><body><h1>%d: Method not allowed.</h1></body></html>", code);
		sprintf(buff, "HTTP/1.1 %d\r\nContent-type: text/html\r\nContent-length: %ld\r\n\r\n%s", code, strlen(content), content);
		send(sock, buff, strlen(buff), 0);
	}else{
		sprintf(content, "<html><body><h1>ERROR %d</h1></body></html>", code);
		sprintf(buff, "HTTP/1.1 %d\r\nContent-type: text/html\r\nContent-length: %ld\r\n\r\n%s", code, strlen(content), content);
		send(sock, buff, strlen(buff), 0);	
	}
}

void SendList(int csock, char *path, char *request){
	if(!dirExist(path)){
		SendError(csock, 404);
		return;
	}

	char *buffer=(char*)malloc(strlen(HTMLDOC)+1);
	buffer[0]=0;
	strcat(buffer, HTMLDOC);
	
	char hostname[HOSNAMETLEN];
    gethostname(hostname, HOSNAMETLEN);
 
	char backPath[NAMELEN]={0};
	getBackDir(backPath, request);

	char *data=getHtmlDataList(path);

	buffer=strrpl(buffer, "###HOST###", hostname);
	buffer=strrpl(buffer, "###WDIR###", request);
	buffer=strrpl(buffer, "###BDIR###", backPath);
	buffer=strrpl(buffer, "###DATA###", data);

	char *sendBuffer=(char*)malloc(512+strlen(buffer));
	sprintf(sendBuffer, "HTTP/1.1 200 OK\nContent-Length: %ld\nContent-Type: text/html\r\n\r\n%s", strlen(buffer), buffer);
	send(csock, sendBuffer, strlen(sendBuffer), 0);

	free(buffer);
	free(data);
	free(sendBuffer);
}

void SendFile(int sock, char *name){
	char buffer[BUFFLEN];

	FILE *file=fopen(name, "rb");
	if(file==NULL){ 
		SendError(sock, 404);
		return;
	}

	fseek(file, 0L, SEEK_END);
	unsigned long size=ftell(file);
 	fseek(file, 0L, SEEK_SET);
 	
 	char type[128];
 	getFileType(type, name); 

	sprintf(buffer, "HTTP/1.1 200 OK\nContent-Length: %ld\nContent-Type: %s\r\n\r\n", size, type);
	send(sock, buffer, strlen(buffer), 0);
	
	while(1){
		int n=fread(buffer, 1, BUFFLEN, file);
		int s=send(sock, buffer, n, 0);
		if(n<BUFFLEN || s<1) break;
	}
	fclose(file);
}

char* getHtmlDataList(char *dir){	
	char *htmlList=(char*)calloc(1, 1);
	int htmlLen=0;
	
	File* files=listFiles(dir);

	File *f=files;
	while(f!=NULL){
		char size[16];
		sizeToH(f->size, size, 15);
			
		const char *icn=getFileType(f->name, NULL);  
	
		char tmp[BUFFLEN];
		
		if(f->isDir){ 
			sprintf(tmp,"<tr>\n<td>\n <a href=\"%s/\"> <img src=\"/res/%s.png\">&nbsp %s</a> \n</td>\n<td> %s </td>\n</tr>\n\n", f->name, "dir", f->name, "DIR");
		}else if (f->isExec){ 
			sprintf(tmp,"<tr>\n<td>\n <a href=\"%s\"> <img src=\"/res/%s.png\">&nbsp %s</a> \n</td>\n<td> %s </td>\n</tr>\n\n", f->name, "exe", f->name, size);
		}else{
			sprintf(tmp,"<tr>\n<td>\n <a href=\"%s\"> <img src=\"/res/%s.png\">&nbsp %s</a> \n</td>\n<td> %s </td>\n</tr>\n\n", f->name, icn, f->name, size);
		}

		htmlLen += strlen(tmp)+1;
		htmlList=(char*)realloc(htmlList, htmlLen*sizeof(char));
		strncat(htmlList, tmp, htmlLen);
		f=f->next;	
	}

	freeFiles(files);
	return htmlList;
}

void getBackDir(char* backDir, char *path){
	int l=strlen(path);
	backDir[0]=0;
	strcat(backDir, path);
	
	if(backDir[l-1]=='/' && (l-1)>0) backDir[l-1]=0;	
	
	while(backDir[l-1]!='/' && l>0) --l;	
	backDir[l]=0;	
}

char* strrpl(char *str, char* old, char* new){
	int strLen=strlen(str);
	int oldLen=strlen(old);
	int newLen=strlen(new);

	str=(char*)realloc(str, strLen+1-oldLen+newLen);

	int i=0;
	while(strncmp(str+i, old, oldLen)!=0){
		++i;
		if(i>=strLen) return str;	
	}

	char *pom=(char*)malloc(strLen-i-oldLen+1);
	pom[0]=0;
	strcat(pom, str+i+oldLen);

	strcpy(str+i, new);
	strcpy(str+i+newLen, pom);

	return str;
}
		
