#include <stdlib.h>
#include <pthread.h>

typedef struct player{
    int socket;
    int r;
    int g;
    int b;
    int state;  // 0 - Inactive Player
                // 1 - Active Player
    pthread_t trd;
    int play[2];
    int ignore;
    pthread_mutex_t statelock;
    pthread_mutex_t ignorelock;
    int corr;
}player;

typedef struct player_node{
    player* p;
    struct player_node* next;
}player_node;