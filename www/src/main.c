#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include "confuse.h"
#include "ip.h"
#include "http.h"
#include "types.h"
#include "conf.h"
#include "threadpool.h"

#define MAX 20
#define MAXPATH 200

int soc;
cfg_t* conf = NULL;
threadpool* tp;
BOOL father = FALSE;
size_t nchilds;
int *childs;
int acceptfd;

/**
 * @brief Handler en caso de que llegue un SIGINT (ctrl+c)
 * 
 */
void sigint_handler();

/**
 * @brief Demoniza el proceso
 * 
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int to_demonize();

/**
 * @brief Crea los procesos hijos que se encargarán de aceptar las peticiones
 * 
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int run_http();

int main(int argv, char** argc){
    int port, ret, i=0, j=0;
    struct sockaddr_in servaddr;
    char *ip;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    ret = 0;
   
    system("clear");//Limpiamos la terminal 

    conf = get_conf();
    if(!conf) return EXIT_FAILURE;

    ip = cfg_getstr(conf, "ip");
    if(strcmp(ip,"Default") == 0) ip = getIP(cfg_getstr(conf,"interface"));
    printf("IP:%s\n",ip);

    //Demonizamos el proceso
    if(to_demonize()){
        cfg_free(conf);
        return EXIT_FAILURE;  //Creo que funciona pero no estoy seguro.
    }

    nchilds = cfg_getint(conf, "childs");
    if(nchilds > MAX){
        syslog(LOG_INFO, "Too many childs, reduced to %d child(s).", MAX);
        nchilds = MAX;
    }
    childs = (int*)malloc(nchilds*sizeof(int));
    if(!childs){
        syslog(LOG_ERR, "Malloc error.\n");

    }

    port = cfg_getint(conf, "port");
    //Creamos el "dict" con la información del .conf

    /****Creacion de socket****/
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc < 0){
        syslog(LOG_ERR, "Socket creation failed.\n");
        cfg_free(conf);
        free(childs);
        return EXIT_FAILURE;
    }
    else syslog(LOG_INFO, "Socket successfully created...\n");

    //En caso de que llegue un SIGINT cerramos el socket
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        syslog(LOG_ERR, "Signal catcher failed.\n");
        close(soc);
        cfg_free(conf);
        free(childs);
        return EXIT_FAILURE;
    }


    ret = inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);
    if(!ret){
        syslog(LOG_ERR, "Can not connect to IP.\n");
        close(soc);
        cfg_free(conf);
        free(childs);
        return EXIT_FAILURE;
    }

    /****Asignamos una direccion IP y un PORT****/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    /****Asignamos el socket la direccion IP y el PORT****/
    if ((bind(soc, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        syslog(LOG_ERR, "Socket bind failure.\n");
        close(soc);
        cfg_free(conf);
        free(childs);
        return EXIT_FAILURE;
    }
    else
        syslog(LOG_INFO, "Socket successfully binded..\n");   
    
    /****El servidor espera un paquete****/
     if ((listen(soc, 100)) != 0) {
        syslog(LOG_ERR, "Socket listen failure.\n");
        close(soc);
        cfg_free(conf);
        free(childs);
        return EXIT_FAILURE;
    }
    else
        syslog(LOG_INFO, "Server listening..\n");


    /****El servidor acepta al cliente****/
    //####################### VARIOS HILOS ########################//
    //threadpool_start(tp, soc);
    //#############################################################//

    //####################### MULTI PROCESO #######################//
    if(run_http() == EXIT_FAILURE){
        syslog(LOG_ERR, "Childs creation failure.\n");
        for(i=0; i<nchilds; i++) kill(childs[i], SIGINT);
        close(soc);
        cfg_free(conf);
        free(childs);
    }
    //#############################################################//

    //######################## UNI PROCESO ########################//
    /*struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    while(1){
        acceptfd = accept(soc, (struct sockaddr*)&cli, &len);
        if (acceptfd < 0) {
            syslog(LOG_ERR, "Socket accept failure.\n");
            exit(EXIT_FAILURE);
        }
        http(acceptfd);
        close(acceptfd);
    }*/
    //#############################################################//


    close(soc);
    cfg_free(conf);
    free(childs);
    syslog(LOG_INFO, "Finishing program.\n");
    return EXIT_SUCCESS;
}

void sigint_handler(){
    int i;
    if(father == TRUE){
        for(i=0; i<nchilds; i++) kill(childs[i], SIGINT);
        for(i=0; i<nchilds; i++) wait(NULL);
        close(soc);
        free(childs);
        syslog(LOG_INFO, "Finishing program.\n");
    }
    else{
        close(acceptfd);
        wait(NULL);
    }
    exit(EXIT_SUCCESS);
}

int run_http(){
    int i;
    pid_t pid;
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    for(i=0; i<nchilds; i++){
        if((pid = fork()) == -1) return EXIT_FAILURE;
        if(pid == 0){
            while(1){
                acceptfd = accept(soc, (struct sockaddr*)&cli, &len);
                if (acceptfd < 0) {
                    syslog(LOG_ERR, "Socket accept failure.\n");
                    exit(EXIT_FAILURE);
                }
                http(acceptfd);
                close(acceptfd);
            }
            exit(EXIT_SUCCESS);
        }
        else childs[i] = pid;
    }
    father = TRUE;
    for(i=0; i<nchilds; i++) wait(NULL);
    return EXIT_SUCCESS;
}

int to_demonize(){
    pid_t pid = fork();
    char cwd[MAXPATH];

    if(pid < 0) return EXIT_FAILURE;
    if (pid > 0) exit(EXIT_SUCCESS);
    printf("ID: %d\n", getpid());
    umask(S_IRUSR);     //Da los permisos pero es peligroso por ser un riesgo de seguridad (permite modificar ficheros creados por el proceso)
                        //pero como no abrimos ningún tipo de fichero pues por ahora no es peligroso
                        //

    openlog("First practice REDES2 sever: ", LOG_NOWAIT | LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "Init function demonize.\n");

    if(setsid() < 0){           //Crea una nueva sesión con el proceso como lider
        syslog(LOG_ERR, "Not able to create a session.\n");
        exit(EXIT_FAILURE);
    }
    
    if(chdir(getcwd(cwd, sizeof(cwd))) < 0){         //Asigna el directorio base como el actual (www)
        syslog(LOG_ERR, "Not able to change process's directory.\n");
        exit(EXIT_FAILURE);
    }
    //Cerramos las salidas estandar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    return EXIT_SUCCESS;
}