#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include "confuse.h"
#include "ip.h"
#include "http.h"
#include "conf.h"

#define MAX 80

int soc;

int sigint_handler(){
    free(soc);
    exit(EXIT_SUCCESS);
}


int main(int argv, char** argc){
    
    int connfd, port, ret;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
    char *ip;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    ret = 0;
    cfg_t* conf = NULL;
   
    conf = get_conf();
    port = cfg_getint(conf, "port");
    

    /****Creacion de socket****/
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc < 0){
        perror("socket creation failed.\n");
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        close(soc);
        perror("signal");
        exit(1);
    }

    ip = cfg_getstr(conf, "ip");

    if(strcmp(ip,"Default") == 0) ip = getIP(cfg_getstr(conf,"interface"));


    ret = inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);

    if(!ret)
    {
        perror("can not connect to IP.\n");
        return EXIT_FAILURE;
    }


    /****Asignamos una direccion IP y un PORT****/
    servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   //asignacion IP --> localhost
    servaddr.sin_port = htons(port);                //asignacion PORT --> 8080

    /****Asignamos el socket la direccion IP y el PORT****/
    if ((bind(soc, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        perror("socket bind failed...\n");
        close(soc);
        return EXIT_FAILURE;
    }
    else
        fprintf(stdout, "Socket successfully binded..\n");   
    
    /****El servidor espera un paquete****/
     if ((listen(soc, 5)) != 0) {
        perror("Listen failed...\n");
        close(soc);
        return EXIT_FAILURE;
    }
    else
        perror("Server listening..\n");
    len = sizeof(cli);

    /****El servidor acepta al cliente****/
    connfd = accept(soc, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        perror("Server accept failed...\n");
        return EXIT_FAILURE;
    }
    else
        perror("Server accept the client...\n");

    http(connfd);

    close(connfd);
    close(soc);
    
    return EXIT_FAILURE;
}

