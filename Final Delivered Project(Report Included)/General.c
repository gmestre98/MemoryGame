#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "General.h"

/** serverinputs: Function that verifies if the arguments introduced by the user are valid
 * \param argc - Number of arguments
 * \param argv - Arguments
*/
int serverinputs(int argc, char *argv[]){
    int dim;

    if(argc < 2){
        printf("Invalid number of arguments, specify the board size!\n");
        exit(-1);
    }
    dim = atoi(argv[1]);
    if(dim%2 != 0  || dim < 2 || dim >26){
        printf("Introduce a valid board size, being pair, greater then 1, and under 26!\n");
        exit(-1);
    }
    return dim;
}


/** socketserver: Function that initializes the socket on the server side
 * \param sock_fd - Pointer to the socket file descriptor
*/
void socketserver(int *sock_fd){
    struct sockaddr_in local_addr;
    int backlog = 1;

    *sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (*sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }
    local_addr.sin_family = AF_INET;
    local_addr.sin_port= htons(MEMPORT);
    local_addr.sin_addr.s_addr= INADDR_ANY;
    int err = bind(*sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if(err == -1) {
        perror("bind");
        exit(-1);
    }
    listen(*sock_fd, backlog);
}


/** clientinputs: Function that verifies if the number of arguments introduced by the user is valid
 * \param argc - Number of arguments introduced by the user
*/
void clientinputs(int argc){
    if(argc < 2){
        printf("problem in server address inputed\n");
        exit(-1);
    }
}


/** socketclient: Function that initializes the socket on the client side
 * \param sock_fd - Pointer to the socket file descriptor
 * \param argv - Arguments introduced
*/
void socketclient(int *sock_fd, char *argv[]){
    struct sockaddr_in server_socket;

    *sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sock_fd == -1){
        perror("socket: ");
        exit(-1);
    }
    server_socket.sin_family = AF_INET; //tipo
    server_socket.sin_port = htons(MEMPORT); //port
    inet_aton(argv[1], &server_socket.sin_addr);
    if(connect(*sock_fd,
			(const struct sockaddr *) &server_socket,
			sizeof(server_socket)) == -1){
                printf("Connecting error\n");
                exit(-1);
            }
}


/** verifyalloc: Function that verifies if the memory was correctly allocated
 * \param arg - Variable for which the memory was allocated
*/
void verifyalloc(void *arg){
    if(arg == NULL){
        printf("Memory allocation failed!\n");
        exit(-1);
    }
}