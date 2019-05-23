#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "memserver.h"

/* Global Variables */
int dim;
int sock_fd;
player_node *phead = NULL;
int totalplayers = 0;
int activeplayers = 0;
int done = 0;

int main(int argc, char *argv[]){
    SDL_Event event;
    struct sockaddr_in local_addr;
    int backlog = 1;
    const char server_title[7] = "Server";
	const char * window_title = server_title;
    pthread_t logins;
    player_node *aux = phead;
    int newgame = 0;
    char c;

    /* Input verifications */
    if(argc < 2){
        printf("Invalid number of arguments, specify the board size!\n");
        exit(-1);
    }
    dim = atoi(argv[1]);
    if(dim%2 != 0  || dim < 2 || dim >26){
        printf("Introduce a valid board size, being pair, greater then 1, and under 26!\n");
        exit(-1);
    }

    /* Setting up a socket, doing the respective bind and start listening */
    sock_fd= socket(AF_INET, SOCK_STREAM, 0);
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

    /* Waiting for at least 2 players to login */
    printf("Waiting for at least 2 players to login!\n");
    pthread_create(&logins, NULL, newplayers, NULL);
    while(activeplayers <  2){
        sleep(1);
    }

    /* Starting SDL and creating the board */
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
	

    /* Main Loop */
    while(!done){
        create_board_window(300, 300,  dim, window_title);
	    init_board(dim);
        aux = phead;
        while(activeplayers > 0){
            while(SDL_PollEvent(&event)){
                switch(event.type){
                    case SDL_QUIT: {
                        serverkill();
                        activeplayers = 0;
                        done = 1;
                        close_board_windows();
                        while(aux != NULL){
                            pthread_cancel(aux->p->trd);
                            close(aux->p->socket);
                            aux = aux->next;
                        }
                        exit(-1);
                    }
                }
            }
        }
        while(aux != NULL){
            pthread_cancel(aux->p->trd);
            //pthread_join(aux->p->trd, NULL);
            close(aux->p->socket);
            aux = aux->next;
        }
        close_board_windows();
        do{
            printf("Do you want to start a new game?\n");
            printf("Insert the corresponding number: 1-yes 2-no\n");
            c = getchar();
            sscanf(&c, "%d", &newgame);
            if(newgame == 2)
                done = 1;
        }while(newgame < 1  ||  newgame > 2);
        if(newgame == 2)
            break;
        newgame = 0;
        totalplayers = 0;
        while(activeplayers <  2){
            sleep(1);
        }
    }
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
    player_node *auxplayer;


    while(endgame == 0){
        /* Receiving the plays from the client */
        recv(p->socket, bp, sizeof(bp), 0);
        if(bp->x == -1  &&  bp->y == -1)
            break;
        if(bp->x == resp->play1[0] && bp->y == resp->play1[1] && resp->code == 1)
            continue;
        
        *resp = board_play(bp->x, bp->y, p->play); /* Analysing the play */
        resp->r = p->r;
        resp->g = p->g;
        resp->b = p->b;

        /* Sending back to the clients to print */
        auxplayer = phead;
        while(auxplayer != NULL){
            send(auxplayer->p->socket, &(*resp), sizeof(*resp),0);
            auxplayer = auxplayer->next;
        }

        /* Starting to count the time for a first play */
        if(resp->code == 1){
            aux->resp = resp;
            aux->p = p;
            pthread_create(&timerthread, NULL, *timerfplay, (void*)aux);
        }
        if(resp->code != 1){ /* Killing the time count */
            pthread_cancel(timerthread);
        }

        /* Printing the results on the server */
        endgame = print_response_server(*resp, p);
        if(endgame == 1){
            auxplayer = phead;
            while(auxplayer != NULL){
                send(auxplayer->p->socket, &(*resp), sizeof(*resp),0);
                auxplayer = auxplayer->next;
                if(p->trd != aux->p->trd){
                    pthread_cancel(aux->p->trd);
                    aux->p->state = 0;
                }
            }
            activeplayers = 1;
            break;
        }

        /* Note: This could be done with just sending the answer to the client late but
        then there would be a delay of 2 seconds for the plays in which there is a mistake*/
    }
    p->state = 0;
    activeplayers = activeplayers - 1;
    return 0;
}

/** print_response_server: Function that prints all the changes on the board on the server
 * \param resp - response to the player action 
 * \param p - pointer to the player profile
*/
int print_response_server(play_response resp, player* p){
    int endgame=0;
    switch (resp.code) {
			case 1: /* First play */
                savethecolor(p->r, p->g, p->b, resp.play1[0], resp.play1[1]);
				paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b, dim);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200, dim);
				break;
			case 3:/* End Game */
                savethecolor(p->r, p->g, p->b, resp.play1[0], resp.play1[1]);
                savethecolor(p->r, p->g, p->b, resp.play2[0], resp.play2[1]);
    			paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b, dim);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0, dim);
    			paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b, dim);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0, dim);
			    endgame = 1;
                return endgame;
			case 2:/* Second play matching the pieces */
                savethecolor(p->r, p->g, p->b, resp.play1[0], resp.play1[1]);
                savethecolor(p->r, p->g, p->b, resp.play2[0], resp.play2[1]);
    			paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b, dim);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0, dim);
    			paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b, dim);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0, dim);
				break;
			case -2:/* Second play with different pieces */
                savethecolor(p->r, p->g, p->b, resp.play1[0], resp.play1[1]);
                savethecolor(p->r, p->g, p->b, resp.play2[0], resp.play2[1]);
				paint_card(resp.play1[0], resp.play1[1] , p->r, p->g, p->b, dim);
				write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0, dim);
				paint_card(resp.play2[0], resp.play2[1] , p->r, p->g, p->b, dim);
				write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0, dim);
				sleep(2);
				paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255, dim);
				paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255, dim);
                freethepiece(resp.play1[0], resp.play1[1]);
                freethepiece(resp.play2[0], resp.play2[1]);
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
    player_node *auxplayer = phead;
    prev.play1[0] = aux->resp->play1[0];
    prev.play1[1] = aux->resp->play1[1];
    sleep(5);
    if(aux->resp->code == 1 && prev.play1[0] == aux->resp->play1[0] && 
    prev.play1[1] == aux->resp->play1[1]){
        freethepiece(aux->resp->play1[0], aux->resp->play1[1]);
        paint_card(aux->resp->play1[0], aux->resp->play1[1] , 255, 255, 255, dim);
        getbackfirst(aux->p->play);
        aux->resp->code = -1;
        while(auxplayer != NULL){
            send(auxplayer->p->socket, &(*resp), sizeof(*resp),0);
            auxplayer = auxplayer->next;
        }
    }
    return 0;
}

void*newplayers(){
    player_node *head = phead;
    player_node *aux;
    int auxsock;

    if(head == NULL){
        auxsock = accept(sock_fd, NULL, NULL);
        head = get_pnode();
        head->p = newplayer(auxsock);
        activeplayers = activeplayers + 1;
        totalplayers = totalplayers + 1;
        aux = head;
    }
    phead = head;

    while(totalplayers < 64){
        auxsock = accept(sock_fd, NULL, NULL);
        aux->next = get_pnode();
        aux->next->p = newplayer(auxsock);
        activeplayers = activeplayers + 1;
        totalplayers = totalplayers + 1;
        aux = aux->next;
    }
    return 0;
}

player_node* get_pnode(){
    player_node* aux;

    aux = (player_node *)malloc(sizeof(player_node));
    aux->next = NULL;
    return aux;
}

player* newplayer(int auxsock){
    player* p = (player *)malloc(sizeof(player));
    p->socket = auxsock;
    p->r = 255 - 85*totalplayers/16;
    p->g = 255 - 85*totalplayers/4;
    p->b = 255 - 85*totalplayers%4;
    p->play[0] = -1;
    p->state = 1;
    send(p->socket, &dim, sizeof(dim), 0);
    pthread_create(&(p->trd), NULL, *playerfunc, (void*)p);
    return p;
}

void serverkill(){
    player_node *aux = phead;
    play_response *resp = (play_response *)malloc(sizeof(play_response));
    resp->code = -3;

    while(aux != NULL){
        send(aux->p->socket, resp, sizeof(play_response), 0);
        aux = aux->next;
    }
}