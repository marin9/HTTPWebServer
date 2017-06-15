#include <unistd.h>
#include "malloc.h"
#include "net.h"


void web(int sock, char *dir);
void RecvRequest(int csock, char *buffer, int blen, char *c_ip, int ip_len);
int isFinish(char *buff, int sum, int n);
int GetRequest(char *buff, int blen);
void SendError(int sock, int code);
void SendList(int csock, char *path, char* request);
void SendFile(int sock, char *name);
char* getHtmlDataList(char *dir);
void getBackDir(char* backDir, char *path);
char* strrpl(char *str, char* old, char* new);
