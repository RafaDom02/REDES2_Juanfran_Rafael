#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include "confuse.h"
#include "ip.h"
#include "http.h"
#include "conf.h"
#include "threadpool.h"

#define MAX 80
#define MAXPATH 200
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
int to_demonize();

int main(int argv, char** argc){
    int connfd, port, ret, i=0, j=0;
    struct sockaddr_in servaddr, cli;
    socklen_t len = sizeof(cli);
    char *ip;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    ret = 0;
    cfg_t* conf = NULL;
   
    system("clear");//Limpiamos la terminal 

    conf = get_conf();

    ip = cfg_getstr(conf, "ip");
    if(strcmp(ip,"Default") == 0) ip = getIP(cfg_getstr(conf,"interface"));
    printf("IP:%s\n",ip);

    //Demonizamos el proceso
    if(to_demonize()) return EXIT_FAILURE;  //Creo que funciona pero no estoy seguro.

    port = cfg_getint(conf, "port");
    //Creamos el "dict" con la información del .conf

    tp = threadpool_create(cfg_getint(conf, "n_threads"));  //Creación threadpool
    syslog(LOG_INFO, "Threadpool created.\n");

    /****Creacion de socket****/
    soc = socket(AF_INET, SOCK_STREAM, 0);
    if(soc < 0){
        syslog(LOG_ERR, "Socket creation failed.\n");
        return EXIT_FAILURE;
    }
    else syslog(LOG_INFO, "Socket successfully created...\n");

    //En caso de que llegue un SIGINT cerramos el socket
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        syslog(LOG_ERR, "Signal catcher failed.\n");
        close(soc);
        exit(1);
    }


    ret = inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);
    if(!ret){
        syslog(LOG_ERR, "Can not connect to IP.\n");
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
        threadpool_destroy(tp);
        return EXIT_FAILURE;
    }
    else
        syslog(LOG_INFO, "Socket successfully binded..\n");   
    
    /****El servidor espera un paquete****/
     if ((listen(soc, 100)) != 0) {
        syslog(LOG_ERR, "Socket listen failure.\n");
        close(soc);
        threadpool_destroy(tp);
        return EXIT_FAILURE;
    }
    else
        syslog(LOG_INFO, "Server listening..\n");


    /****El servidor acepta al cliente****/
    //####################### VARIOS HILOS ########################//
    //threadpool_start(tp, soc);
    //#############################################################//

    //###################### UN SOLO PROCESO ######################//
    while(1){
        connfd = accept(soc, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            syslog(LOG_ERR, "Socket accept failure.\n");
            close(soc);
            return EXIT_FAILURE;
        }
        http(connfd);
        close(connfd);
    }
    //#############################################################//

    close(soc);
    threadpool_destroy(tp);
    syslog(LOG_INFO, "Finishing program.\n");
    return EXIT_SUCCESS;
}

void sigint_handler(){
    close(soc);
    threadpool_destroy(tp);
    syslog(LOG_INFO, "Finishing program.\n");
    exit(EXIT_SUCCESS);
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