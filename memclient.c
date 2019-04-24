#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>

#include "UI_library.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

int sock_fd;


int main(int argc, char * argv[]){ //gets the server IP from argv
    SDL_Event event;
	int done = 0;
    int dim;
    struct sockaddr_in server_socket;


    //Prevent invalid input arguments
    if(argc < 2){
        printf("problem in server address inputed\n");
        exit(-1);
    }


    // Criação do socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }

    //Atribuição do address do socket
    server_socket.sin_family = AF_INET; //tipo
    server_socket.sin_port = htons(MEMPORT); //port
    inet_aton(argv[1], &server_socket.sin_addr);

    // Connect do socket
    if(connect(sock_fd,
			(const struct sockaddr *) &server_socket,
			sizeof(server_socket)) == -1){
                printf("Connecting error\n");
                exit(-1);
            }

    //Request ao server pela dimensão
    recv(sock_fd, &dim, sizeof(dim), 0);


//Start creating the window
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}

	create_board_window(300, 300,  dim);
	init_board(dim);

//Create the mouse event caption
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
					//play_response resp = board_play(board_x, board_y); || Send board_x board_y to server
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