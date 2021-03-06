#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "UI_library.h"
#include "General.h"

int sock_fd;
int done = 0;
int dim;
int activegame = 0;
int endgame = 0;

void *sendpos(void* arg);
void *recv_from_server();
void *exitthread();

int main(int argc, char * argv[]){
	/* The expected argument is the server IP */
    SDL_Event event;
	pthread_t thread_send;
	pthread_t thread_recv;
	boardpos* bp = (boardpos *)malloc(sizeof(boardpos));
	const char client_title[7] = "Client";
	const char * window_title = client_title;
	pthread_t exitt;

    clientinputs(argc);
	socketclient(&sock_fd, argv);
    recv(sock_fd, &dim, sizeof(dim), 0);
	StartingSDL();
	create_board_window(300, 300,  dim, window_title);
	pthread_create(&thread_recv, NULL, *recv_from_server, NULL);


	/* SDL Events caption */
	while (!done){
		while(activegame == 0)
			sleep(1);
		endgame = 0;
		while(endgame == 0){
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT: {
						done = SDL_TRUE;
						pthread_create(&exitt, NULL, exitthread, NULL);
						endgame = 1;
						break;
					}
					case SDL_MOUSEBUTTONDOWN:{
						if(activegame == 1){
							get_board_card(300/dim, 300/dim, event.button.x, event.button.y, &(bp->x), &(bp->y));
							printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, bp->x, bp->y);
							pthread_create(&thread_send, NULL, *sendpos, (void*) bp);
						}
					}
				}
			}
		}
		close_board_windows();
		sleep(8);
		create_board_window(300, 300,  dim, window_title);
	}
	printf("The client was closed!\n");
	free(bp);
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
	piece *p = (piece*)malloc(sizeof(piece));

	while(1){
		recv(sock_fd, p, sizeof(*p), 0);
		if(p->wr == 255  &&  p->wg == 255  &&  p->wb == 255)
			paint_card(p->x, p->y, p->wr, p->wg, p->wb, dim);
		else if(p->wr == 0  &&  p->wg == 255  &&  p->wb == 0)
			activegame = 1;
		else if(p->wr == 0  &&  p->wg == 0  &&  p->wb == 255)
			activegame = 0;
		else if(p->wr == 0  &&  p->wg == 255  &&  p->wb == 255){
			endgame = 1;
			done = 1;
			printf("The server crashed!\n");
			break;
		}
		else{
			paint_card(p->x, p->y, p->pr, p->pg, p->pb, dim);
			printf("pr:%d, pg:%d, pb:%d, wr:%d, wg:%d, wb:%d\n", p->pr, p->pg, p->pb, p->wr, p->wg, p->wb);
			write_card(p->x, p->y, p->str, p->wr, p->wg, p->wb, dim);
		}
		if(p->end != 0){
			printf("The game ended, in ten seconds a new window will appear!\n");
			activegame = 0;
			sleep(2);
			endgame = 1;
		}
	}
	return 0;
	free(p);
}

/** recv_from_server: Function that receives information from the server
*/
void *exitthread(){
	boardpos bp;
	bp.x = -1;
	bp.y = -1;
	send(sock_fd, &bp, sizeof(boardpos), 0);
	return 0;
}