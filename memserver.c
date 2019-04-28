#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "board_library.h"
#include "UI_library.h"

void *playerfunc(void*);
int print_response_server(play_response, player*);
void *timerfplay(void *arg);

int main(){
    //FAZER AINDA: IMPLEMENTAR LEITURA DO TAMANHO DO TABULEIRO POR ARGUMENTO NA MAIN
    SDL_Event event;
    struct sockaddr_in local_addr;
    int backlog = 4;
    int totalusers = 0;
    int auxsock;
    int dim = 4;
    const char server_title[7] = "Server";
	const char * window_title = server_title;

    /* Setting up a socket, doing the respective bind and start listening */
    int sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }
    local_addr.sin_family = AF_INET;
    local_addr.sin_port= htons(MEMPORT);
    local_addr.sin_addr.s_addr= INADDR_ANY;
    int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if(err == -1) {
        perror("bind");
        exit(-1);
    }
    listen(sock_fd, backlog);

    /* Accepting a client and setting up his "profile", sending the board dimension
    and creating a thread for that client */
    auxsock = accept(sock_fd, NULL, NULL);
    player* p = (player*)malloc(sizeof(player));
    p->socket = auxsock;
    p->r = 0;
    p->g = 0;
    p->b = 255;
    send(p->socket, &dim, sizeof(dim), 0);
    pthread_create(&(p->trd), NULL, *playerfunc, (void*)p);

    /* Starting SDL and creating the board */
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
	create_board_window(300, 300,  dim, window_title);
	init_board(dim);


    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT: {
                // FAZER AINDA: Criar thread enviar erro e mandar tudo pelos ares
	            close_board_windows();
                exit(-1);
            }
        }
    }
    pthread_join(p->trd, NULL);
    close(p->socket);
	close_board_windows();
}

/** playerfunc: Function that does all the processing related to one client,
 * receiving the plays, analysing them and then printing and sending the
 * answer back to the client 
 * \param arg - pointer to that player profile
*/
void *playerfunc(void *arg){
    player* p = (player*) arg;
    boardpos *bp = (boardpos *)malloc(sizeof(boardpos));
    int endgame = 0;
    pthread_t timerthread;
    play_response *resp = (play_response *)malloc(sizeof(play_response));
    respplayer *aux = (respplayer *)malloc(sizeof(respplayer));
    

    while(endgame == 0){
        /* Receiving the plays from the client */
        recv(p->socket, bp, sizeof(bp), 0);
        if(bp->x == -1  &&  bp->y == -1)
            break;
        if(bp->x == resp->play1[0] && bp->y == resp->play1[1] && resp->code == 1)
            continue;
        /* Analysing the plays */
        *resp = board_play(bp->x, bp->y); 
        resp->r = p->r;
        resp->g = p->g;
        resp->b = p->b;

        /* Sending back to the client to print */
        send(p->socket, &(*resp), sizeof(*resp),0);

        /* Starting to count the time for a first play */
        if(resp->code == 1){
            aux->resp = resp;
            aux->p = p;
            pthread_create(&timerthread, NULL, *timerfplay, (void*)aux);
        }

        /* Printing the results on the server */
        endgame = print_response_server(*resp, p);
        if(endgame == 1)
            send(p->socket, &(*resp), sizeof(*resp),0);

        /* Note: This could be done with just sending the answer to the client late but
        then there would be a delay of 2 seconds for the plays in which there is a mistake*/
    }
    // FAZER AINDA: Quando so der para jogar com pelo menos 2 clients diminuir a contagem
}

/** print_response_server: Function that prints all the changes on the board on the server
 * \param resp - response to the player action 
 * \param p - pointer to the player profile
*/
int print_response_server(play_response resp, player* p){
    int endgame=0;
    switch (resp.code) {
			case 1: /* First play */
				paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
				break;
			case 3:/* End Game */
			  endgame = 1;
              return endgame;
			case 2:/* Second play matching the pieces */
    			paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
    			paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
				break;
			case -2:/* Second play with different pieces */
				paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
				paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
				sleep(2);
				paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
				paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
				break;
		}
    return endgame;
}

/** timerfplay: Function that implements the timer for the first play, blocking the
 * piece only for five seconds  
 * \param arg - Structure containing the response to the player action and the player
 * profile
*/
void*timerfplay(void *arg){
    respplayer *aux = (respplayer *)arg;
    play_response *resp =  aux->resp;
    play_response prev;
    prev.play1[0] = aux->resp->play1[0];
    prev.play1[1] = aux->resp->play1[1];
    sleep(5);
    if(aux->resp->code == 1 && prev.play1[0] == aux->resp->play1[0] && 
    prev.play1[1] == aux->resp->play1[1]){
        paint_card(aux->resp->play1[0], aux->resp->play1[1] , 255, 255, 255);
        getbackfirst();
        aux->resp->code = -1;
        send(aux->p->socket, &(*resp), sizeof(*resp), 0);
    }
}