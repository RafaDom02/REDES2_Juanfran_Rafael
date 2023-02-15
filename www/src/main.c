#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "http.h"
#include "confuse.h"

#define MAX 80
#define PORT 8081


int main(int argv, char** argc){
    
    int soc, connfd, len;
    struct sockaddr_in servaddr, cli;

    /****Creacion de socket****/
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc < 0){
        perror("socket creation failed.\n");
        return EXIT_FAILURE;
    }
    inet_pton(AF_INET, "192.168.218.26", &servaddr.sin_addr.s_addr);


    /****Asignamos una direccion IP y un PORT****/
    servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //asignacion IP --> localhost
    servaddr.sin_port = htons(PORT);                //asignacion PORT --> 8080

    /****Asignamos el socket la direccion IP y el PORT****/
    if ((bind(soc, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        perror("socket bind failed...\n");
        exit(0);
    }
    else
        fprintf(stdout, "Socket successfully binded..\n");   
    
    /****El servidor espera un paquete****/
     if ((listen(soc, 5)) != 0) {
        perror("Listen failed...\n");
        exit(0);
    }
    else
        perror("Server listening..\n");
    len = sizeof(cli);

    /****El servidor acepta al cliente****/
    connfd = accept(soc, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        perror("Server accept failed...\n");
        exit(0);
    }
    else
        perror("Server accept the client...\n");

    http(connfd);

    close(connfd);
    close(soc);
    
}

