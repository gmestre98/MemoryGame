#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "UI_library.h"
#include "board_library.h"
#include "players_library.h"

int sock_fd;

void *sendpos(void* arg);

int main(int argc, char * argv[]){ //gets the server IP from argv
    SDL_Event event;
	int done = 0;
    int dim;
    struct sockaddr_in server_socket;
	pthread_t thread_send;
	boardpos* bp = (boardpos *)malloc(sizeof(boardpos));

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

//Create the mouse event caption
	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					get_board_card(300/dim, 300/dim, event.button.x, event.button.y, &(bp->x), &(bp->y));
					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, bp->x, bp->y);
					pthread_create(&thread_send, NULL, *sendpos, (void*) bp);
				}
			}
		}
	}
	printf("fim\n");
	close_board_windows();
	//Notify the server about the quiting
}

void *sendpos(void *arg){
	boardpos bp = *(boardpos*) arg;
	printf("boardpos x:%d \t y:%d\n", bp.x, bp.y);
	send(sock_fd, &(bp), sizeof(bp),0);
}