#include <stdlib.h>
#include <pthread.h>

typedef struct board_place{
  char v[3];
  int state;  // 0 - Free
              // 1 - 1st Play
              // 2 - Taken
              // -2 - Mistake
  int r;
  int g;
  int b;
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -3 Server killed
            // -2 2nd - diffrent
            // -1 Get First piece down for some reason
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
  int r;
  int g;
  int b;
} play_response;


typedef struct play_node{
  int play1[2];
  struct play* next;
}play_node;

char * get_board_place_str(int i, int j);
void init_board(int dim);
void getbackfirst(int [2]);
play_response board_play (int x, int y, int [2]);
void freethepiece(int, int);
void savethecolor(int, int, int, int, int);
int checkboardstate(int, int);
int getboardcolor(int, int, int);
int checkboardnull();