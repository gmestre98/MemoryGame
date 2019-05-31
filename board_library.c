#include <stdlib.h>
#include "board_library.h"
#include <stdio.h>
#include <string.h>

int dim_board;
board_place * board;
int play1[2];
int n_corrects;

/** linearconv: Function that converts the coordinates of a 2 dimensional
 * matrix into the equivalent of the board vector 
 * \param i - one of the coordinates of the matrix
 * \param j - the other coordinate of the matrix
*/
int linear_conv(int i, int j){
  return j*dim_board+i;
}

/** get_board_place_str: Function that gets the string on some board position
 * \param i - one of the coordinates of the matrix
 * \param j - the other coordinate of the matrix
*/
char * get_board_place_str(int i, int j){
  return board[linear_conv(i, j)].v;
}

/** getbackfirst: Function that resets the plays
*/
void getbackfirst(int play1[2]){
  play1[0] = -1;
}

/** init_board: Function that initializes the board pieces
 * \param dim - dimension of the board
*/
void init_board(int dim){
  int count  = 0;
  int i, j;
  char * str_place; 

  dim_board= dim;
  n_corrects = 0;
  board = malloc(sizeof(board_place)* dim *dim);

  for(i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
    board[i].state = 0;
  }

  for (char c1 = 'a' ; c1 < ('a'+dim_board); c1++){
    for (char c2 = 'a' ; c2 < ('a'+dim_board); c2++){
      do{
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board*dim_board)
        return;
    }
  }
}


/** board_play: Function that analyses a play and generating the respective response
 * \param x - one of the coordinates of the matrix
 * \param y - the other coordinate of the matrix
 * \return - play_response containing the information regarding the play made by the user
*/
play_response board_play(int x, int y, int play1[2]){
  play_response resp;
  resp.code =10;
  if(board[linear_conv(x, y)].state != 0  &&  play1[0] != -1){ 
    printf("FILLED\n");
    resp.code = -1;
    resp.play1[0]= play1[0];
    resp.play1[1]= play1[1];
    board[linear_conv(play1[0], play1[1])].state = 0;
    play1[0] = -1;
  }
  else if(board[linear_conv(x, y)].state != 0  &&  play1[0] == -1){
    printf("FILLED\n");
    resp.code = 0;
  }
  else{
    if(play1[0]== -1){
        printf("FIRST\n");
        resp.code =1;

        play1[0]=x;
        play1[1]=y;
        resp.play1[0]= play1[0];
        resp.play1[1]= play1[1];
        strcpy(resp.str_play1, get_board_place_str(x, y));
        board[linear_conv(x, y)].state = 1;
    }
    else{
        char * first_str = get_board_place_str(play1[0], play1[1]);
        char * secnd_str = get_board_place_str(x, y);

        if ((play1[0]==x) && (play1[1]==y)){
          resp.code =0;
          printf("FILLED\n");
        }
        else{
          resp.play1[0]= play1[0];
          resp.play1[1]= play1[1];
          strcpy(resp.str_play1, first_str);
          resp.play2[0]= x;
          resp.play2[1]= y;
          strcpy(resp.str_play2, secnd_str);

          if (strcmp(first_str, secnd_str) == 0){
            printf("CORRECT!!!\n");
            board[linear_conv(play1[0], play1[1])].state = 2;
            board[linear_conv(x, y)].state = 2;
            n_corrects +=2;
            if (n_corrects == dim_board* dim_board)
                resp.code =3;
            else
              resp.code =2;
          }
          else{
            printf("INCORRECT\n");
            board[linear_conv(play1[0], play1[1])].state = -2;
            board[linear_conv(x, y)].state = -2;
            resp.code = -2;
          }
          play1[0]= -1;
        }
    }
  }
  return resp;
}


/** freethepiece: Function that makes some piece state to be 0
 * \param x - one of the coordinates of the matrix for that piece
 * \param y - the other coordinate of the matrix for that piece
*/
void freethepiece(int x, int y){
  board[linear_conv(x, y)].state = 0;
}


/** savethecolor: Function that saves the color of the player that owns some piece
 * on the board position corresponding to the piece
 * \param r - red component of rgb for that player color
 * \param g - green component of rgb for that player color
 * \param b - blue component of rgb for that player color
 * \param x - one of the coordinates of the matrix for that piece
 * \param y - the other coordinate of the matrix for that piece
*/
void savethecolor(int r, int g, int b, int x, int y){
  board[linear_conv(x, y)].r = r;
  board[linear_conv(x, y)].g = g;
  board[linear_conv(x, y)].b = b;
}

/** checkboardstate: Function that returns the state of the selected piece
 * \param x - one of the coordinates of the matrix
 * \param y - the other coordinate of the matrix
 * \return - state of the board
*/
int checkboardstate(int x, int y){
  return board[linear_conv(x, y)].state;
}


/** getboardcolor: Function that returns a color component for some piece of the board
 * \param x - one of the coordinates of the matrix
 * \param y - the other coordinate of the matrix
 * \param color - parameter that gives the color component to be returned
 * \return - 1:red;  2:green;  3:blue
*/
int getboardcolor(int x, int y, int color){
  switch(color){
    case 1:
      return board[linear_conv(x, y)].r;
      break;
    case 2:
      return board[linear_conv(x, y)].g;
      break;
    case 3:
      return board[linear_conv(x, y)].b;
      break;
  }
  return 0;
}


/** checkboardnull: Function that verifies if the board is null
 * \return - 0 if it is null, 1 if it is not
*/
int checkboardnull(){
  if(board == NULL)
    return 0;
  return 1;
}