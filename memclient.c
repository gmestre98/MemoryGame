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

int sock_fd;
int done = 0;
int dim;

void *sendpos(void* arg);
void *recv_from_server();
void *exitthread();

int main(int argc, char * argv[]){
	/* The expected argument is the server IP */
    SDL_Event event;
    struct sockaddr_in server_socket;
	pthread_t thread_send;
	pthread_t thread_recv;
	boardpos* bp = (boardpos *)malloc(sizeof(boardpos));
	const char client_title[7] = "Client";
	const char * window_title = client_title;
	pthread_t exitt;


    /* Prevents invalid number of arguments */
    if(argc < 2){
        printf("problem in server address inputed\n");
        exit(-1);
    }


	/* Socket creation */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }

    /*Atribution of the socket addres and respective connect */
    server_socket.sin_family = AF_INET; //tipo
    server_socket.sin_port = htons(MEMPORT); //port
    inet_aton(argv[1], &server_socket.sin_addr);
    if(connect(sock_fd,
			(const struct sockaddr *) &server_socket,
			sizeof(server_socket)) == -1){
                printf("Connecting error\n");
                exit(-1);
            }

    /* Receiving the board dimension from the server */
    recv(sock_fd, &dim, sizeof(dim), 0);
   

	/* Starting the SDL window */
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
	create_board_window(300, 300,  dim, window_title);

	/* Creating a thread that receives info from the server */
	pthread_create(&thread_recv, NULL, *recv_from_server, NULL);

	/* SDL Events caption */
	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
					pthread_create(&exitt, NULL, exitthread, NULL);
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


/** sendpos: Function that sends a position from the board where the player pressed
 * \param arg - structure with the x and the y position of the press
*/
void *sendpos(void *arg){
	boardpos bp = *(boardpos*) arg;
	printf("boardpos x:%d \t y:%d\n", bp.x, bp.y);
	send(sock_fd, &(bp), sizeof(bp),0);
	return 0;
}

/** recv_from_server: Function that receives information from the server
*/
void *recv_from_server() {
	play_response *resp = (play_response *)malloc(sizeof(play_response));
	int endgame = 0;

	while(endgame == 0){
		recv(sock_fd, resp, sizeof(*resp), 0);/* receives the response from the server
		(already interpreted by the server) */
		switch (resp->code) {
			case 1: /* First play */
				printf("CASE 1\n");
				paint_card(resp->play1[0], resp->play1[1] , resp->r, resp->g, resp->b, dim);
				write_card(resp->play1[0], resp->play1[1], resp->str_play1, 200, 200, 200, dim);
				break;
			case 3:/* End Game */
				printf("CASE 3\n");
				endgame = 1;
				done = 1;
				break;
			case 2:/* Second play matching the pieces */
				printf("CASE 2\n");
				paint_card(resp->play1[0], resp->play1[1] , resp->r, resp->g, resp->b, dim);
				write_card(resp->play1[0], resp->play1[1], resp->str_play1, 0, 0, 0, dim);
				paint_card(resp->play2[0], resp->play2[1] , resp->r, resp->g, resp->b, dim);
				write_card(resp->play2[0], resp->play2[1], resp->str_play2, 0, 0, 0, dim);
				break;
			case -2:/* Second play with different pieces */
				printf("CASE -2\n");
				paint_card(resp->play1[0], resp->play1[1] , resp->r, resp->g, resp->b, dim);
				write_card(resp->play1[0], resp->play1[1], resp->str_play1, 255, 0, 0, dim);
				paint_card(resp->play2[0], resp->play2[1] , resp->r, resp->g, resp->b, dim);
				write_card(resp->play2[0], resp->play2[1], resp->str_play2, 255, 0, 0, dim);
				sleep(2);
				paint_card(resp->play1[0], resp->play1[1] , 255, 255, 255, dim);
				paint_card(resp->play2[0], resp->play2[1] , 255, 255, 255, dim);
				break;
			case -1:/* First play 5 seconds ended without another piece pressed */
				paint_card(resp->play1[0], resp->play1[1] , 255, 255, 255, dim);
				paint_card(resp->play2[0], resp->play2[1] , 255, 255, 255, dim);
				break;
		}
	}
	return 0;
}

void *exitthread(){
	boardpos bp;
	bp.x = -1;
	bp.y = -1;
	send(sock_fd, &bp, sizeof(boardpos), 0);
	return 0;
}