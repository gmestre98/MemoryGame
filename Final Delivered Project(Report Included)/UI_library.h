#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


typedef struct piece{
    int x;
    int y;
    int pr;
    int pg;
    int pb;
    char str[3];
    int wr;
    int wg;
    int wb;
    int end;
}piece;

typedef struct boardpos{
	int x;
	int y;
}boardpos;

void write_card(int  board_x, int board_y, char * text, int r, int g, int b, int dim);
void paint_card(int  board_x, int board_y , int r, int g, int b, int dim);
void clear_card(int  board_x, int board_y, int dim);
void get_board_card(int col_width, int row_height, int mouse_x, int mouse_y, int * board_x, int *board_y);
int create_board_window(int width, int height,  int dim, const char *);
void close_board_windows();
void StartingSDL();