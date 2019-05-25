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

static int filled_index = -1; //-1 means empty
static boardpos* filled_vector;

boardpos* botplay_coordinates;
pthread_t thread_send;

void *sendpos(void* arg);
void *recv_from_server();
void *exitthread();
void get_botplay_coordinates();
void push_filledvector(boardpos*);
int check_if_filled();

int main(int argc, char * argv[]){
	/* The expected argument is the server IP */
    SDL_Event event;
    struct sockaddr_in server_socket;
	pthread_t thread_recv;
	botplay_coordinates = (boardpos *)malloc(sizeof(boardpos));
	const char client_title[4] = "Bot";
	const char * window_title = client_title;
	pthread_t exitt;

	srand(time(NULL)); 

    /* Prevents invalid number of arguments */
    if(argc < 2){
        printf("problem in server address inputed\n");
        exit(-1);
    }

	// FAZER AINDA: TALVEZ VERIFICAR SE É MESMO UM IP QUE ESTÁ AQUI A SER COLOCADO

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
	filled_vector = (boardpos*)malloc((dim*dim)*sizeof(boardpos));
	filled_index = 0;
   

	/* Starting the SDL window */
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
	pthread_create(&thread_recv, NULL, *recv_from_server, NULL);


	/* SDL Events caption */
	while (!done){
		while(activegame == 0)
			sleep(1);
		create_board_window(300, 300,  dim, window_title);
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
		sleep(8);
	}
	printf("The client was closed!\n");
	exit(-1);
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
	int start_sending_botplays = 0;

	while(1){
		if(start_sending_botplays == 1){
			get_botplay_coordinates(botplay_coordinates);
			pthread_create(&thread_send, NULL, *sendpos, (void*) botplay_coordinates);
		}
		recv(sock_fd, p, sizeof(*p), 0);
		if(p->wr == 255  &&  p->wg == 255  &&  p->wb == 255)
			paint_card(p->x, p->y, p->wr, p->wg, p->wb, dim);
		else if(p->wr == 0  &&  p->wg == 255  &&  p->wb == 0){
			activegame = 1;
			start_sending_botplays = 1;
		}
		else if(p->wr == 0  &&  p->wg == 0  &&  p->wb == 255)
			activegame = 0;
		else if(p->wr == 0  &&  p->wg == 255  &&  p->wb == 255){
			endgame = 0;
			done = 1;
			printf("The server crashed!\n");
			break;
		}
		else{
			paint_card(p->x, p->y, p->pr, p->pg, p->pb, dim);
			printf("pr:%d, pg:%d, pb:%d, wr:%d, wg:%d, wb:%d\n", p->pr, p->pg, p->pb, p->wr, p->wg, p->wb);
			write_card(p->x, p->y, p->str, p->wr, p->wg, p->wb, dim);
			if(p->wr == 0 && p->wg == 0 && p->wb == 0)
				push_filledvector(botplay_coordinates);

		}
		if(p->end != 0){
			printf("The game ended, in ten seconds a new window will appear!\n");
			activegame = 0;
			sleep(2);
			endgame = 1;
		}
	}
	return 0;
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


/** get_botplay_coordinates: This Function generates coordinates that DO NOT
 *  match with ANY filled piece
*/
void get_botplay_coordinates(){

	while(1)
	{
		botplay_coordinates->x = rand() % dim; //if dim = 4, the x goes from 0 to 3
		botplay_coordinates->y = rand() % dim;
		if(check_if_filled() == 0)
			break;
	}

	sleep(1); //give some time to the other client's plays

}

/** check_if_filled: This Function returns 1 if the played coordinates are the same
 * as a filled piece
*/
int check_if_filled(){
	int aux = filled_index;

	while(aux >= 0)
	{
		if(	filled_vector[aux].x == botplay_coordinates->x && filled_vector[aux].y == botplay_coordinates->y )
		{
			return 1; //already filled
		}
		aux--;
	}
	return 0;
}

/** push_filledvector: This Function "pushes" the play coordinates to the filled vector,
 * increasing the filled_index.
 * \param bp - Bot played coordinates
*/
void push_filledvector(boardpos * bp){

	filled_vector[filled_index].x = bp->x;
	filled_vector[filled_index].y = bp->y;
	filled_index++;
}
