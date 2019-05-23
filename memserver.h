#include "multiplayer.h"
#include "board_library.h"
#include "UI_library.h"

typedef struct respplayer{
  play_response *resp;
  player *p;
}respplayer;


/* Functions Declaration */
void *playerfunc(void*);
int print_response_server(play_response, player*);
void *timerfplay(void*);
void *newplayers();
player_node *get_pnode();
player *newplayer(int);
void serverkill();