#include <stdlib.h>

typedef struct board_place{
  char v[3];
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
} play_response;

char * get_board_place_str(int i, int j);
void init_board(int dim);
play_response board_play (int x, int y);
