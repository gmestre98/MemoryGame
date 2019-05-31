#include "multiplayer.h"
#include "board_library.h"
#include "UI_library.h"

typedef struct respplayer{
  play_response *resp;
  player *p;
}respplayer;


/* Functions Declaration */
void *playerfunc(void*);
void dealwithresp(play_response *, player *);
piece *sendpiecetoclient(player *, int [2], char *, int, int, int, int);
piece *producepiece(player *, int [2], char *, int, int, int, int);
void print_piece(piece *);
int print_response_server(play_response, player*);
void *timerfplay(void*);
void *cleanpiece(void*);
void *stopignore(void*);
void *newplayers();
player_node *get_pnode();
player *newplayer(int);
void serverkill();
void prepare_client_exit(player*, respplayer *);
void mutexinit();
void mutexdestroy();
void invasionentry(player *);