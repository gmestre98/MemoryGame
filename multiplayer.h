#include <stdlib.h>

typedef struct player{
    int socket;
    int r;
    int g;
    int b;
    pthread_t trd;
}player;

typedef struct player_node{
    player* p;
    struct player_node* next;
}player_node;