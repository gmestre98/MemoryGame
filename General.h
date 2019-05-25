#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MEMPORT 3000

int serverinputs(int, char **);
void socketserver(int *);
void clientinputs(int);
void socketclient(int *, char **);