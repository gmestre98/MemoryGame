#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MEMPORT 3000

void write_card(int  board_x, int board_y, char * text, int r, int g, int b, int dim);
void paint_card(int  board_x, int board_y , int r, int g, int b, int dim);
void clear_card(int  board_x, int board_y, int dim);
void get_board_card(int col_width, int row_height, int mouse_x, int mouse_y, int * board_x, int *board_y);
int create_board_window(int width, int height,  int dim, const char *);
void close_board_windows();
