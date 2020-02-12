#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>

#include "board_library.h"
#include "UI_library.h"

int main(){

	SDL_Event event;
	int done = 0;

	 if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}


	create_board_window(300, 300,  4);
	init_board(4);

	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					int board_x, board_y;
					get_board_card(event.button.x, event.button.y, &board_x, &board_y);

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					play_response resp = board_play(board_x, board_y);
					switch (resp.code) {
						case 1:
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
							break;
						case 3:
						  done = 1;
						case 2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
							break;
						case -2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
							sleep(2);
							paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
							paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
							break;
					}
				}
			}
		}
	}
	printf("fim\n");
	close_board_windows();
}
