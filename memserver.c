#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "memserver.h"
#include "General.h"


/* Global Variables */
int dim;
int sock_fd;
player_node *phead = NULL;
int totalplayers = 0;
int activeplayers = 0;
int done = 0;
int endgame = 0;
char empty[1] = "e";
pthread_mutex_t *locker;
pthread_mutex_t lockergraphic;

int main(int argc, char *argv[]){
    SDL_Event event;
    const char server_title[7] = "Server";
	const char * window_title = server_title;
    pthread_t logins;

    dim = serverinputs(argc, argv);
    socketserver(&sock_fd);
    mutexinit();    
    printf("Waiting for at least 2 players to login!\n");
    pthread_create(&logins, NULL, newplayers, NULL);
    StartingSDL();

    /* Main Loop */
    while(!done){
	    init_board(dim);
        while(activeplayers <  2){
            sleep(1);
        }
        create_board_window(300, 300,  dim, window_title);
        endgame = 0;
        sendpiecetoclient(NULL, NULL, NULL, 0, 0, 255, 0);
        while(endgame == 0){
            while(SDL_PollEvent(&event)){
                switch(event.type){
                    case SDL_QUIT: {
                        serverkill();
                        exit(-1);
                    }
                }
            }
        }
        sendpiecetoclient(NULL, NULL, NULL, 0, 0, 0, 255);
        close_board_windows();
        sleep(10);
    }
    mutexdestroy();
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
    play_response *resp = (play_response *)malloc(sizeof(play_response));
    respplayer *aux = (respplayer *)malloc(sizeof(respplayer));
    char *recvBuff = malloc(sizeof(boardpos));
    

    while(1){
        /* Receiving the plays from the client */
        recv(p->socket, recvBuff, sizeof(boardpos), 0);
        memcpy(bp, recvBuff, sizeof(boardpos));
        if(p->ignore == 1)
            continue;
        if(bp->x == -1  &&  bp->y == -1){
            prepare_client_exit(p);
            break;
        }
        if(bp->x == resp->play1[0] && bp->y == resp->play1[1] && resp->code == 1)
            continue;
        
        pthread_mutex_lock(&locker[bp->x]);
        *resp = board_play(bp->x, bp->y, p->play); /* Analysing the play */
        pthread_mutex_unlock(&locker[bp->x]);
        resp->r = p->r;
        resp->g = p->g;
        resp->b = p->b;

        /* Starting to count the time for a first play */
        if(resp->code == 1){
            aux->resp = resp;
            aux->p = p;
            pthread_create(&timerthread, NULL, *timerfplay, (void*)aux);
        }
        else if(resp->code != 0)/* Killing the time count */
            pthread_cancel(timerthread);

        // Verificar se faz sentido dar lock nesta zona
        if(resp->code != 0)
            dealwithresp(resp, p);
    }
    pthread_cancel(timerthread);
    free(recvBuff);
    free(bp);
    free(resp);
    free(aux);
    return 0;
}


/** dealwithresp: This Function deals with the play response, answering accordingly
 * to the code of it
 * \param resp - Pointer to the play response
 * \param p - Pointer to the player profile
*/
// Verficar se faz sentido o savethecolor e o sendpiecetoclient serem regiões criticas
void dealwithresp(play_response *resp, player *p){
    piece *peca;
    pthread_t trdclean[3];

    switch (resp->code){
            case 0:
                break;
            case 1:
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 200, 200, 200);
                print_piece(peca);
                free(peca);
                break;
            case 2:
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 0, 0, 0);
                print_piece(peca);
                free(peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 0, 0, 0, 0);
                print_piece(peca);
                free(peca);
                break;
            case 3: /* End Game */
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 0, 0, 0);
                print_piece(peca);
                free(peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 1, 0, 0, 0);
                print_piece(peca);
                free(peca);
                endgame = 1;
                break;
            case -2:
                p->ignore = 1;
                savethecolor(p->r, p->g, p->b, resp->play1[0], resp->play1[1]);
                savethecolor(p->r, p->g, p->b, resp->play2[0], resp->play2[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 255, 0, 0);
                print_piece(peca);
                pthread_create(&(trdclean[0]), NULL, cleanpiece, (void *)peca);
                peca = sendpiecetoclient(p, resp->play2, resp->str_play2, 0, 255, 0, 0);
                print_piece(peca);
                pthread_create(&(trdclean[1]), NULL, cleanpiece, (void *)peca);
                pthread_create(&(trdclean[2]), NULL, stopignore, (void *)p);
                break;
            case -1: /* Free one piece */
                freethepiece(resp->play1[0], resp->play1[1]);
                peca = sendpiecetoclient(p, resp->play1, resp->str_play1, 0, 255, 255, 255);
                print_piece(peca);
                free(peca);
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
    piece *peca;
    char *data = malloc(sizeof(piece));
    
    peca = producepiece(p, play, str, e, wr, wg, wb);
    memcpy(data, peca, sizeof(piece));
    //printf("Size:%d\n", sizeof(piece));
    auxplayer = phead;
    while(auxplayer != NULL){
        if(auxplayer->p != NULL  &&  auxplayer->p->state == 1)
            send(auxplayer->p->socket, data, sizeof(piece),0);
        auxplayer = auxplayer->next;
    }
    free(data);
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
    if(p != NULL){
        peca->pr = p->r;
        peca->pg = p->g;
        peca->pb = p->b;
    }
    else{
        peca->pr = 0;
        peca->pg = 0;
        peca->pb = 0;
    }
    if(play != NULL){ 
        peca->x = play[0];
        peca->y = play[1];
    }
    else{
        peca->x = -1;
        peca->x = -1;
    }
    if(str != NULL)
        strcpy(peca->str, str);
    else{
        peca->str[0] = 'x';
        peca->str[1] = 'x';
        peca->str[2] = '\0';
    }
    peca->wr = wr;
    peca->wg = wg;
    peca->wb = wb;

    return peca;
}

/** print_piece: Function that prints the updated information of some piece
 * \param p - Piece with the updated information
*/
void print_piece(piece *p){
    pthread_mutex_lock(&lockergraphic);
    if(p->wr == 255  &&  p->wg == 255  &&  p->wb == 255){
		paint_card(p->x, p->y, p->wr, p->wg, p->wb, dim);
    }
	else{
		paint_card(p->x, p->y, p->pr, p->pg, p->pb, dim);
		write_card(p->x, p->y, p->str, p->wr, p->wg, p->wb, dim);
	}
    pthread_mutex_unlock(&lockergraphic);
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
    //printf("x: %d, y:%d\n", peca->x, peca->y);
    freethepiece(peca->x, peca->y);
    free(peca);
    peca = sendpiecetoclient(&p, play, peca->str, 0, 255, 255, 255);
    print_piece(peca);
    free(peca);
    return 0;
}

/** stopignore: Function that implements the 2 seconds ignore for the player that failed a play
 *  \param arg - Player profile
*/
void *stopignore(void *arg){
    player *p = (player *)arg;

    sleep(2);
    p->ignore = 0;
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
    piece *peca;
    char *data1 = malloc(sizeof(int));
    char *data2 = malloc(sizeof(piece));

    verifyalloc((void *)p);
    verifyalloc((void *)data1);
    verifyalloc((void *)data2);

    p->socket = auxsock;
    p->r = 255 - 85*(totalplayers/16);
    p->g = 255 - 85*(totalplayers/4);
    p->b = 255 - 85*(totalplayers%4);
    //printf("r:%d, g:%d, b:%d\n", p->r, p->g, p->b);
    p->play[0] = -1;
    p->state = 1;
    p->ignore = 0;
    memcpy(data1, &dim, sizeof(int));
    //printf("Size:%d\n", sizeof(int));
    send(p->socket, data1, sizeof(int), 0);
    free(data1);
    if(activeplayers == 1){
        phead->p->ignore = 0;
        peca = producepiece(NULL, NULL, NULL, 0, 0, 255, 0);
        memcpy(data2, peca, sizeof(piece));
        //printf("Size:%d\n", sizeof(piece));
        send(p->socket, data2, sizeof(piece), 0);
        if(phead->p->ignore != 0)
            send(phead->p->socket, data2, sizeof(piece), 0);
        free(data2);
        free(peca);
    }
    else if(activeplayers > 1){
        phead->p->ignore = 0;
        peca = producepiece(NULL, NULL, NULL, 0, 0, 255, 0);
        memcpy(data2, peca, sizeof(piece));
        //printf("Size:%d\n", sizeof(piece));
        send(p->socket, data2, sizeof(piece), 0);
        free(data2);
        free(peca);
    }
    else
        p->ignore = 1;
    invasionentry(p);
    pthread_create(&(p->trd), NULL, *playerfunc, (void*)p);
    return p;
}


/** serverkill - Function that make the necessary conditions so the server can be killed
 * killing all the clients
*/
void serverkill(){
    player_node *aux = phead;
    
    sendpiecetoclient(NULL, NULL, NULL, 0, 0, 255, 255);
    activeplayers = 0;
    done = 1;
    close_board_windows();
    aux = phead;
    while(aux != NULL){
        pthread_cancel(aux->p->trd);
        close(aux->p->socket);
        free(aux->p);
        aux = aux->next;
    }
    close(sock_fd);
}


/** prepare_client_exit - Function that makes the necessary arrangements when a client
 * leaves the game
 * \param p - Player who exited the game
*/
void prepare_client_exit(player* p){
    player_node *aux = phead;
    piece *peca;
    char *data = malloc(sizeof(piece));
  
    p->state = 0;
    activeplayers = activeplayers - 1;
    if(activeplayers == 1){
        while(aux != NULL  &&  aux->p->state != 1)
            aux = aux->next;
        aux->p->ignore = 1;
        peca = producepiece(NULL, NULL, NULL, 0, 0, 0, 255);
        memcpy(data, peca, sizeof(piece));
        //printf("Size:%d\n", sizeof(piece));
        send(aux->p->socket, data, sizeof(piece), 0);
    }
}


/** mutexinit - Function that initializes all the necessary mutexes for the server
*/
void mutexinit(){
    locker = (pthread_mutex_t *)malloc(dim * sizeof(pthread_mutex_t));
    verifyalloc((void *)locker);
    for(int i=0; i < dim; i = i + 1){
        if(pthread_mutex_init(&(locker[i]), NULL) != 0){
            printf("\n mutex init %d failed \n", i);
            exit(-1);
        }
    }
    if(pthread_mutex_init(&(lockergraphic), NULL) != 0){
        printf("\n mutex init %d failed \n", i);
        exit(-1);
    }
}

/** mutexinit - Function that destroys all mutexes
*/
void mutexdestroy(){
    for(int i=0; i < dim; i = i + 1){
        pthread_mutex_destroy(&locker[i]);
    }
    pthread_mutex_destroy(&lockergraphic);
}


/** invasionentry - Function the board to a new client when it enters in the middle of the game
 * \param p - Player profile
*/
void invasionentry(player *p){
    int play[2];
    player auxp;
    piece *peca;
    char *data = malloc(sizeof(piece));
    verifyalloc((void *)data);

    for(int x=0; x < dim; x = x + 1){
        for(int y=0; y < dim; y = y + 1){
            if(checkboardnull()  &&  checkboardstate(x, y) != 0){
                play[0] = x;
                play[1] = y;
                auxp.r = getboardcolor(x, y, 1);
                auxp.g = getboardcolor(x, y, 2);
                auxp.b = getboardcolor(x, y, 3);
                switch (checkboardstate(x, y)){
                    case 1:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 200, 200, 200);
                        memcpy(data, peca, sizeof(piece));
                        //printf("Size:%d\n", sizeof(piece));
                        send(p->socket, data, sizeof(piece), 0);
                        free(peca);
                        break;
                    case 2:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 0, 0, 0);
                        memcpy(data, peca, sizeof(piece));
                        //printf("Size:%d\n", sizeof(piece));
                        send(p->socket, data, sizeof(piece), 0);
                        free(peca);
                        break;
                    case -2:
                        peca = producepiece(&auxp, play, get_board_place_str(x, y), 0, 255, 0, 0);
                        memcpy(data, peca, sizeof(piece));
                        //printf("Size:%d\n", sizeof(piece));
                        send(p->socket, data, sizeof(piece), 0);
                        free(peca);
                        break;
                }
            }
        }
    }
}