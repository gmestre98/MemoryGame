#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "UI_library.h"
#include "General.h"

int sock_fd;
int done = 0;
int dim;
int activegame = 0;
int activesend = 0;
int endgame = 0;
int **pieces;

void *sendpos(void* arg);
void *recv_from_server();
void *exitthread();
void *send_plays();
void getcoordinates(boardpos *);
int filled(int, int);
void *reactivate();

int main(int argc, char * argv[]){
	/* The expected argument is the server IP */
    SDL_Event event;
	pthread_t thread_send;
	pthread_t thread_recv;
	const char client_title[7] = "Client";
	const char * window_title = client_title;
	pthread_t exitt;

    clientinputs(argc);
	socketclient(&sock_fd, argv);
    recv(sock_fd, &dim, sizeof(int), 0);
	StartingSDL();
	create_board_window(300, 300,  dim, window_title);
	pthread_create(&thread_recv, NULL, *recv_from_server, NULL);
	pthread_create(&thread_send, NULL, send_plays, NULL);
    srand(time(NULL));

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
				}
			}
		}
		close_board_windows();
        if(!done){
		    sleep(8);
		    create_board_window(300, 300,  dim, window_title);
        }
	}
	printf("The client was closed!\n");
}


/** sendpos: Function that sends a position from the board where the player pressed
 * \param arg - structure with the x and the y position of the press
*/
void *sendpos(void *arg){
	boardpos bp = *(boardpos*) arg;
	printf("boardpos x:%d \t y:%d\n", bp.x, bp.y);
	send(sock_fd, &(bp), sizeof(boardpos),0);
	return 0;
}

void *send_plays(){
	int milisec = 100;
	struct timespec req = {0};
	boardpos* bp = (boardpos *)malloc(sizeof(boardpos));
    pieces = (int**)malloc(sizeof(int*)*dim);
    for(int i=0; i < dim; i = i + 1){
        pieces[i] = (int*)malloc(sizeof(int)*dim);
        for(int j=0; j < dim; j = j + 1)
            pieces[i][j] = 0;
    }
	req.tv_sec = 0;
	req.tv_nsec = milisec *1000000L;

    while(1){
        if(!activegame){
            sleep(1);
            continue;
        }
        getcoordinates(bp);
        sendpos((void *)bp);
		nanosleep(&req, (struct timespec *)NULL);
    }
    free(bp);
    for(int i=0; i < dim; i = i + 1){
        free(pieces[i]);
    }
    free(pieces);
}

/** recv_from_server: Function that receives information from the server
*/
void *recv_from_server() {
	piece *p = (piece*)malloc(sizeof(piece));
    pthread_t treactivate;

	while(1){
		recv(sock_fd, p, sizeof(piece), 0);
		if(p->wr == 255  &&  p->wg == 255  &&  p->wb == 255)
			paint_card(p->x, p->y, p->wr, p->wg, p->wb, dim);
		else if(p->wr == 0  &&  p->wg == 255  &&  p->wb == 0){
			activegame = 1;
            activesend = 1;
        }
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
            if(p->wr == 0  &&  p->wg == 0  &&  p->wb == 0)
                pieces[p->x][p->y] = 1;
            else if(p->wr == 255  &&  p->wg == 0  &&  p->wb == 0){
                activesend = 0;
                pthread_create(&treactivate, NULL, reactivate, NULL);
            }
		}
		if(p->end != 0){
			printf("The game ended, in ten seconds a new window will appear!\n");
			activegame = 0;
            activesend = 0;
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


void getcoordinates(boardpos *bp){   
    do{
        bp->x = random() %dim;
        bp->y = random() %dim;
    }while(filled(bp->x, bp->y));
}

int filled(int x, int y){
    if(pieces[x][y] == 0)
        return 0;
    return 1;
}

void *reactivate(){
    sleep(2);
    activesend = 1;
    return 0;
}