#include <stdlib.h>
#include <pthread.h>

typedef struct player{
    int socket;
    int r;
    int g;
    int b;
    pthread_t trd;
}player;

typedef struct boardpos{
	int x;
	int y;
}boardpos;