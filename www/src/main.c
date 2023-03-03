#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "confuse.h"
#include "ip.h"
#include "http.h"
#include "conf.h"
#include "threadpool.h"

#define MAX 80
#define BASE_DIR "/"

int soc;
threadpool* tp;

/**
 * @brief Handler en caso de que llegue un SIGINT (ctrl+c)
 * 
 */
void sigint_handler();

/**
 * @brief Demoniza el proceso
 * 
 */
void to_demonize();

int main(int argv, char** argc){
    int connfd, port, ret, i=0, j=0;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
    char *ip;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    ret = 0;
    cfg_t* conf = NULL;
   
   //Demonizamos el proceso
   //to_demonize(); //Creo que funciona pero no estoy seguro.

    //Creamos el "dict" con la información del .conf
    conf = get_conf();
    port = cfg_getint(conf, "port");

    tp = threadpool_create(cfg_getint(conf, "n_threads"));  //Creación threadpool

    /****Creacion de socket****/
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc < 0){
        perror("socket creation failed.\n");
        return EXIT_FAILURE;
    }

    //En caso de que llegue un SIGINT cerramos el socket
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
    servaddr.sin_port = htons(port);                //asignacion PORT

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
    //####################### VARIOS HILOS ########################//
    //threadpool_start(tp, soc);
    //#############################################################//

    //###################### UN SOLO PROCESO ######################//
    connfd = accept(soc, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        perror("Server accept failed...\n");
        return EXIT_FAILURE;
    }
    http(connfd);
    close(connfd);
    //#############################################################//

    close(soc);
    
    return EXIT_FAILURE;
}

void sigint_handler(){
    close(soc);
    threadpool_destroy(tp);
    exit(EXIT_SUCCESS);
}

void to_demonize(){
    pid_t pid = fork();

    if(pid < 0) return EXIT_FAILURE;
    if (pid > 0) exit(EXIT_SUCCESS);
    
    //umask(0);     //Da los permisos pero es peligroso por ser un riesgo de seguridad (permite modificar ficheros creados por el proceso)

    if(setsid() < 0) exit(EXIT_FAILURE);    //Crea una nueva sesión con el proceso como lider
    if(chdir("/") < 0) exit(EXIT_FAILURE);  //Asigna el directorio base como el actual (www)

    //Cerramos las salidas estandar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    return;
}