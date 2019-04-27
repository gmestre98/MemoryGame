#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "board_library.h"
#include "UI_library.h"
#include "players_library.h"

void *get_client_responses(void*);
int print_response_server(play_response, player*); //return 1 if the game has ended


int main(){ //Implementar tamanho por  argumento na main
    SDL_Event event;
    struct sockaddr_in local_addr;
    int backlog = 4;
    int totalusers = 0;
    int auxsock;
    int dim = 4;
    const char server_title[7] = "Server"; //SIMAS
	const char * window_title = server_title;

    // Setting up a socket
    int sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }
    local_addr.sin_family = AF_INET;
    local_addr.sin_port= htons(MEMPORT);
    local_addr.sin_addr.s_addr= INADDR_ANY;

    // Doing the bind
    int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if(err == -1) {
        perror("bind");
        exit(-1);
    }
    listen(sock_fd, backlog);

    // Accepting a client and setting up his "profile"
    auxsock = accept(sock_fd, NULL, NULL);
    player* p = (player*)malloc(sizeof(player));
    p->socket = auxsock;
    p->r = 255;
    p->g = 0;
    p->b = 0;
    //printf("%d\n", dim);
    send(p->socket, &dim, sizeof(dim), 0); //give the dimension of the board to the client
    send(p->socket, p, sizeof(p), 0);     //SIMAS - faltava enviar o perfil do player para o respectivo client

    // Starting SDL
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
    
    pthread_create(&(p->trd), NULL, *get_client_responses, (void*)p); //Thread para o respectivo client


    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT: {
                // Criar thread enviar erro e mandar tudo pelos ares
	            close_board_windows();
            }
        }
    }
    pthread_join(p->trd, NULL);
    close(p->socket);
	close_board_windows();
}


void *get_client_responses(void *arg){
    player* p = (player*) arg;
    boardpos *bp = (boardpos *)malloc(sizeof(boardpos)); //get the client's mouse clicked pos's
    int endgame = 0;
    //enviar a cor e caracterizar
    play_response resp; 
    while(1){
        recv(p->socket, bp, sizeof(bp), 0);
        //printf("board crlhasga x:%d \t y:%d\n", bp->x, bp->y);

        resp = board_play(bp->x, bp->y); //interpretar a jogada
        //write card que escreve a carta com a cor para as letras
        //paint que pinta a carta

        //imprimir
        endgame = print_response_server(resp, p);

        //enviar pro client //IMPLEMENTAR PARA TODOS OS CLIENTS DEPOIS
        printf("            B4 sending RESPONSE: %d\n", resp.code);
        printf("  Play1: %d  %d", resp.play1[0], resp.play1[1]);
        printf("  Play2: %d  %d", resp.play2[0], resp.play2[1]);

        send(p->socket, &(resp), sizeof(resp),0);
    }
    //change 2 a do while later - close the thread when the client closes the Window or Winning
}

int print_response_server(play_response resp, player* p){
    int endgame=0;
    switch (resp.code) {
			case 1: //primeira jogada
				paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
				break;
			case 3://fim do jogo
			  endgame = 1;
              return endgame;
			case 2://
    			paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
    			paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
				break;
			case -2:
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
