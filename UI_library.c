#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int screen_width;
int screen_height;
int n_ronw_cols;
int row_height = 300;
int col_width = 300;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

/** write_card: Function that prints the letters of the piece on a card
 * \param x - one of the coordinates of the matrix
 * \param y - the other coordinate of the matrix
 * \param text - the corresponding string containing the letters
 * \param r - red component for the colour to use for the letter print
 * \param g - green component for the colour to use for the letter print
 * \param b - blue component for the colour to use for the letter print
*/
void write_card(int  board_x, int board_y, char * text, int r, int g, int b, int dim){
	SDL_Rect rect;

	rect.x = board_x * (col_width/dim);
	rect.y = board_y * (row_height/dim);
	rect.w = (col_width/dim)+1;
	rect.h = (row_height/dim)+1;

	
	TTF_Font * font = TTF_OpenFont("arial.ttf", row_height);

	//int text_x = board_x * (col_width/dim);
	//int text_y = board_y * (row_height/dim);

	
	SDL_Color color = { r, g, b };
 	SDL_Surface * surface = TTF_RenderText_Solid(font, text, color);
	SDL_Texture* Background_Tx = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); /* we got the texture now -> free surface */

	SDL_RenderCopy(renderer, Background_Tx, NULL, &rect);
	SDL_RenderPresent(renderer);
	SDL_Delay(5);
}

/** paint_card: Function that paints a card with the colour of a player
 * \param x - one of the coordinates of the matrix
 * \param y - the other coordinate of the matrix
 * \param r - red component for the colour to use for the card print
 * \param g - green component for the colour to use for the card print
 * \param b - blue component for the colour to use for the card print
*/
void paint_card(int  board_x, int board_y , int r, int g, int b, int dim){
	SDL_Rect rect;

	rect.x = board_x * (col_width/dim);
	rect.y = board_y * (row_height/dim);
	rect.w = (col_width/dim)+1;
	rect.h = (row_height/dim)+1;
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &rect);

	SDL_RenderPresent(renderer);
	SDL_Delay(5);
}

/** clear_card: Function that cleans a card painting it white
 * \param x - one of the coordinates of the card
 * \param y - the other coordinate of the card
*/
void clear_card(int  board_x, int board_y, int dim){
	paint_card(board_x, board_y , 255, 255, 255, dim);
}

/** get_board_card: Function that gets the coordinates of a card from the pressed
 * coordinates on the mouse
 * \param col_width - width of each card
 * \param row_height - height of each card
 * \param mouse_x - x position the mouse pressed
 * \param mouse_y - y position the mouse pressed
 * \param board_x - pointer to the x coordinate of the board
 * \param board_y - pointer to the y coordinate of the board
*/
void get_board_card(int col_width, int row_height, int mouse_x, int mouse_y, int * board_x, int *board_y){
	*board_x = mouse_x / col_width;
	*board_y = mouse_y / row_height;
}

/** create_board_window: Function that creates a window for the board
 * \param width - width of the window
 * \param height - height of the window
 * \param dim - Number of cards per row or col
 * \param window_title - string with the title of the window
*/
int create_board_window(int width, int height,  int dim, const char *window_title){

	int screen_width = width;
	int screen_height = height;
	int n_ronw_cols = dim;
	int row_height = height /n_ronw_cols;
	int col_width = width /n_ronw_cols;
	screen_width = n_ronw_cols * col_width +1;
	screen_height = n_ronw_cols *row_height +1;


	if (SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer)  != 0) {
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);


	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	for (int i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, 0, i*row_height, screen_width, i*row_height);
	}

	for (int i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, i*col_width, 0, i*col_width, screen_height);
	}
	SDL_RenderPresent(renderer);
	SDL_Delay(5);

	SDL_SetWindowTitle(window, window_title);

	return 0;
}

/** close_board_window - Function that closes the board window
*/
void close_board_windows(){
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
}

void StartingSDL(){
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
}