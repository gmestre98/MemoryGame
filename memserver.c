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
int endgame = 0;

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
        while(activeplayers <  2){
            sleep(1);
        }
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
    pthread_t timerthread;
    play_response *resp = (play_response *)malloc(sizeof(play_response)); //TESTAR METER ISTO SEM PONTEIRO, VARIAVEL LOCAL
    respplayer *aux = (respplayer *)malloc(sizeof(respplayer)); // IGUAL A LINHA DE CIMA
    player_node *auxplayer = phead;
    

    while(endgame == 0){
        /* Receiving the plays from the client */
        recv(p->socket, bp, sizeof(bp), 0);
        if(bp->x == -1  &&  bp->y == -1)
            break;
        if(bp->x == resp->play1[0] && bp->y == resp->play1[1] && resp->code == 1) // TESTAR EVENTUAL RETIRADA, ENTENDER PORQUE ESTÁ AQUI
            continue;
        
        *resp = board_play(bp->x, bp->y, p->play); /* Analysing the play */
        resp->r = p->r;
        resp->g = p->g;
        resp->b = p->b;

        /* Starting to count the time for a first play */
        if(resp->code == 1){
            aux->resp = resp;
            aux->p = p;
            pthread_create(&timerthread, NULL, *timerfplay, (void*)aux);
        }
        else/* Killing the time count */
            pthread_cancel(timerthread);

        dealwithresp(resp, p);
        
        /* Ending the game */
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
    }
    p->state = 0;
    activeplayers = activeplayers - 1;
    return 0;
}


/** dealwithresp: This Function deals with the play response, answering accordingly
 * to the code of it
 * \param resp - Pointer to the play response
 * \param p - Pointer to the player profile
*/
void dealwithresp(play_response *resp, player *p){
    piece *peca = (piece *)malloc(sizeof(piece));
    pthread_t trdclean;

    switch (resp->code){
            case 0:
                break;
            case 1:
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 200, 200, 200);
                print_piece(peca);
                break;
            case 2:
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 0, 0, 0);
                print_piece(peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 0, 0, 0, 0);
                print_piece(peca);
                break;
            case 3: /* End Game */
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 1, 0, 0, 0);
                print_piece(peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 1, 0, 0, 0);
                print_piece(peca);
                endgame = 1;
                break;
            case -2:
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 255, 0, 0);
                print_piece(peca);
                pthread_create(&trdclean, NULL, cleanpiece, (void *)peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 0, 255, 0, 0);
                print_piece(peca);
                pthread_create(&trdclean, NULL, cleanpiece, (void *)peca);
                break;
                // FAZER THREAD PARA ESPERAR DOIS SEGUNDOS E PASSADO DOIS SEGUNDOS LIBERTAR AS PEÇAS
            case -1: /* Free one piece */
                freethepiece(resp->play1[0], resp->play1[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 255, 255, 255);
                print_piece(peca);
                break;
        }
}


/** sendpiecetoclient: Function that sends a piece structure to all clients so the altered
 * piece can be printed, both on the server and on all clients
 * \param p - Pointer to the player profile
 * \param play - Vector with the x and y position of the piece, being the x the position 0
 * and the y the position 2
 * \param str - String with the characters of the piece
 * \param e - Flag being 0 if the game is running and 1 if someone won
 * \param wr - Red component for the color to write on the letters
 * \param wg - Green component for the color to write on the letters
 * \param wb - Blue component for the color to write on the letters
 * return: Produced piece structure
*/
piece *sendpiecetoclient(player *p, int play[2], char *str, int e, int wr, int wg, int wb){
    player_node *auxplayer;
    piece *peca = (piece *)malloc(sizeof(piece));
    
    peca = producepiece(p, play, str, e, wr, wg, wb);
    auxplayer = phead;
    while(auxplayer != NULL){
        send(auxplayer->p->socket, peca, sizeof(*peca),0);
        auxplayer = auxplayer->next;
    }
    return peca;
}


/** producepiece: Function that set's up a piece structure
 * \param p - Pointer to the player profile
 * \param play - Vector with the x and y position of the piece, being the x the position 0
 * and the y the position 2
 * \param str - String with the characters of the piece
 * \param e - Flag being 0 if the game is running and 1 if someone won
 * \param wr - Red component for the color to write on the letters
 * \param wg - Green component for the color to write on the letters
 * \param wb - Blue component for the color to write on the letters
 * return: Produced piece structure
*/
piece *producepiece(player *p, int play[2], char *str, int e, int wr, int wg, int wb){
    piece *peca = (piece *)malloc(sizeof(piece));
    
    peca->end = e;
    peca->pr = p->r;
    peca->pg = p->g;
    peca->pb = p->b;
    peca->x = play[0];
    peca->y = play[1];
    strcpy(peca->str, str);
    peca->wr = wr;
    peca->wg = wg;
    peca->wb = wb;

    return peca;
}

/** print_piece: Function that prints the updated information of some piece
 * \param p - Piece with the updated information
*/
void print_piece(piece *p){
    if(p->wr == 255  &&  p->wg == 255  &&  p->wb == 255)
		paint_card(p->x, p->y, p->wr, p->wg, p->wb, dim);
	else{
		paint_card(p->x, p->y, p->pr, p->pg, p->pb, dim);
		write_card(p->x, p->y, p->str, p->wr, p->wg, p->wb, dim);
	}
}

/** timerfplay: Function that implements the timer for the first play, blocking the
 * piece only for five seconds  
 * \param arg - Structure containing the response to the player action and the player
 * profile
*/
void*timerfplay(void *arg){
    respplayer *aux = (respplayer *)arg;
    play_response prev;

    prev.play1[0] = aux->resp->play1[0];
    prev.play1[1] = aux->resp->play1[1];
    sleep(5);
    if(aux->resp->code == 1 && prev.play1[0] == aux->resp->play1[0] && 
    prev.play1[1] == aux->resp->play1[1]){
        aux->resp->code = -1;
        dealwithresp(aux->resp, aux->p);
        getbackfirst(aux->p->play);
    }
    return 0;
}


/** cleanpiece: Function that cleans a piece after the 2 seconds wrong display time
 * \param arg - Structure of the piece to clean;
*/
void *cleanpiece(void *arg){
    piece *peca = (piece *)arg;
    int play[2];
    player p;

    play[0] = peca->x;
    play[1] = peca->y;
    p.r = peca->pr;
    p.g = peca->pg;
    p.b = peca->pb;
    sleep(2);
    freethepiece(peca->x, peca->y);
    peca = sendpiecetoclient(&p, play, peca->str, 0, 255, 255, 255);
    print_piece(peca);
    return 0;
}


/** newplayers: Function that receives the connection of new players to the game and
 * inserts them accordingly on a list
*/
void*newplayers(){
    player_node *head = phead;
    player_node *aux;
    int auxsock;

    if(head == NULL){ /* First insertion for an empty list */
        auxsock = accept(sock_fd, NULL, NULL);
        head = get_pnode();
        head->p = newplayer(auxsock);
        activeplayers = activeplayers + 1;
        totalplayers = totalplayers + 1;
        aux = head;
    }
    phead = head;

    while(totalplayers < 64){ /* Following insertions */
        auxsock = accept(sock_fd, NULL, NULL);
        aux->next = get_pnode();
        aux->next->p = newplayer(auxsock);
        activeplayers = activeplayers + 1;
        totalplayers = totalplayers + 1;
        aux = aux->next;
    }
    return 0;
}

/** get_pnode: Function that generates a node of the player list
*/
player_node* get_pnode(){
    player_node* aux;

    aux = (player_node *)malloc(sizeof(player_node));
    aux->next = NULL;
    return aux;
}

/** newplayer: Function that generates the color for a newplayer, and attributes all
 * the info to the node on the list. This function also sends back to the client the 
 * board dimension and updates the board accordingly
 * \param auxsock - socket identifier for the communication between the server and
 * the client for that player
*/
player* newplayer(int auxsock){
    player* p = (player *)malloc(sizeof(player));
    player auxp;
    int play[2];
    piece *peca = (piece *)malloc(sizeof(piece));

    p->socket = auxsock;
    p->r = 255 - 85*(totalplayers/16);
    p->g = 255 - 85*(totalplayers/4);
    p->b = 255 - 85*(totalplayers%4);
    printf("r:%d, g:%d, b:%d\n", p->r, p->g, p->b);
    p->play[0] = -1;
    p->state = 1;
    send(p->socket, &dim, sizeof(dim), 0);

    for(int x=0; x < dim; x = x + 1){
        for(int y=0; y < dim; y = y + 1){
            if(checkboardstate(x, y) != 0){
                play[0] = x;
                play[1] = y;
                auxp.r = getboardcolor(x, y, 1);
                auxp.g = getboardcolor(x, y, 2);
                auxp.b = getboardcolor(x, y, 3);
                switch (checkboardstate(x, y)){
                    case 1:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 200, 200, 200);
                        send(p->socket, peca, sizeof(*peca), 0);
                        break;
                    case 2:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 0, 0, 0);
                        send(p->socket, peca, sizeof(*peca), 0);
                        break;
                    case -2:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 255, 0, 0);
                        send(p->socket, peca, sizeof(*peca), 0);
                        break;
                }
            }
        }
    }

    pthread_create(&(p->trd), NULL, *playerfunc, (void*)p);
    return p;
}


// FAZER AINDA: CORRIGIR ISTO
/** serverkill - Function that make the necessary conditions so the server can be killed
 * killing all the clients
*/
/*void serverkill(){
    player_node *aux = phead;
    play_response *resp = (play_response *)malloc(sizeof(play_response));
    resp->code = -3;

    while(aux != NULL){
        send(aux->p->socket, resp, sizeof(play_response), 0);
        aux = aux->next;
    }
}*/