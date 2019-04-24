#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "board_library.h"
#include "UI_library.h"
#include "players_library.h"

void *pfunc();

int main(){ //Implementar tamanho por  argumento na main
    struct sockaddr_in local_addr;
    int backlog = 4;
    int totalusers = 0;
    int auxsock;
    int dim = 4;

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
    printf("oi\n");
    send(sock_fd, &dim, sizeof(dim), 0);
    printf("ja escrevi\n");

    pthread_create(&(p->trd), NULL, *pfunc, NULL);
    printf("Ja fui comido\n");

    // Starting SDL
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

    pthread_join(p->trd, NULL);
	close_board_windows();
}

void *pfunc(){
    printf("Entrei\n");
    sleep(4);
    printf("E agora\n");
}