#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "http.h"
#include "fileparser.h"
#include "types.h"

#define PATH "/www/"

/********
* FUNCIÓN: const char* get_extension(const char *path)
* ARGS_IN: const char* path - path para el fichero pedido
* DESCRIPCIÓN: Devuelve la extensión del fichero incluyendo el .
* ARGS_OUT: const char* - cadena de texto con la extension
********/
const char* get_extension(const char *path){
    char *s;
    if(!path) return NULL;

    s = &(path[strlen(path)-1]);

    for(; path != s && s[0] != '.'; s--);    //Comprobamos que que file no es mayor al path dado o que s[0] sea una .

    if(s[0] == '.') return s;                //Si ha acabado el for-loop con el punto, devuelve la extension
    return NULL;
}

/********
* FUNCIÓN: const char* get_file(const char *path, const char *ext)
* ARGS_IN: const char* path - path para el fichero pedido; const char *ext - extensión del fichero pedido
* DESCRIPCIÓN: Devuelve el nombre del fichero pedido dado un path, ejemplo: /media/images/img7.jpg -> img7.jpg
* ARGS_OUT: const char* - cadena de texto con la extension
********/
const char* get_file(const char *path, const char *ext){
    char *s = strstr(path, ext);
    if(!s) return NULL;

    for(; path != s && s[0] != '/'; s--);   //Comprobamos que que file no es mayor al path dado o que s[0] sea una /

    if(s[0] == '/') return ++s;             //Eliminamos la barra del directorio si ha acabado el for-loop por la barra
    return NULL;
}

/********
* FUNCIÓN: int get_params(char* extension, char** params)
* ARGS_IN: char* extension - nombre con el tipo de extension; char** params - lista de string con los parametos
* DESCRIPCIÓN: Dada una string de argumentod http "?variableGET=hola+mundo+GET 
*              separa los parametros en una array de strings"
* ARGS_OUT: int - número de parámetros, -1 en caso de error
********/
int get_params(char* extension, char** params){
    if(!extension) return -1;
    char* aux = strtok(extension, "=");
    aux = strtok(NULL, "=");
    
    int i = 0;
    char* token = strtok(aux, "+"); 
    while (token != NULL && i < 100) {
        params[i] = (char*)malloc(64*sizeof(char)); 
        strcpy(params[i],token); 
        token = strtok(NULL, "+"); 
        printf("%s\n", params[i]);
        i++;
    }
    return i;
    
}

/********
* FUNCIÓN: char* execute_script(char* path, int* len, char** params, int numparams)
* ARGS_IN: char* path - 
* DESCRIPCIÓN: Ejecuta el script dado por la ruta con sus respectivos argumentos y entrada estandar
* para despues recoger la salida y devolverla
* ARGS_OUT: int - número de parámetros, -1 en caso de error
********/
char* execute_script(char* path, int* len, char** params, int numparams, char* form){
    char * extension, *aux;
    int msglen = 0, i;
    aux = strtok(path, "?");
    extension = get_extension(aux);

    int pipefd[2];
    int readfd[2];
    char def[2] = "";
    char *buf;
    char aux2[BUFLEN];
    char comm[BUFLEN] = "/usr/bin/php";
    char name[BUFLEN] = "php";
    char **args;

    
    
    if (!form){
        form = def;
    }

    buf = (char*)malloc(BUFLEN*sizeof(char));
    if(!buf){
        return NULL;
    }
    
    
    if (strcmp(extension, PY)==0){
        strcpy(comm, "/usr/bin/python3");
        strcpy(name,"python3");
    }

    // create a pipe for communication
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    if (pipe(readfd) == -1) {
        perror("pipe");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(1);
    }


    if (numparams > 0){
        path = strtok(path, "?");
    }
  
    args = (char**)malloc((numparams+3)*sizeof(char*));
    if (!args){
        exit(1);
    }

    args[0]=name;
    strcpy(aux2, path);
    sprintf(path, "./%s", aux2);
    args[1] =path;
    memcpy(args + 2, params, numparams*sizeof(char*));
    args[numparams+2] = NULL;

    strtok(form, "=");
    form = strtok(NULL, "=");

    if (!form){
        form = def;
    }
    
    for (i = 0; i < strlen(form); i++){
        if (form[i] == '+'){
            form[i] = '\n';
        }
    }
    
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        
        close(pipefd[1]);  // close the write end of the pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(readfd[0]);
        dup2(readfd[1], STDOUT_FILENO); 
        execv(comm, args);
        
        
        perror("execl");
        exit(1);
    } else {
        // parent process - send input to the script and read output from the script
        close(pipefd[0]);  // close the read end of the pipe
        close(readfd[1]);
        strcat(form, "\n");
        write(pipefd[1], form, strlen(form)); // send input to the script
        wait(NULL);
        *len = read(readfd[0], buf, BUFLEN);
        close(pipefd[1]);
        close(readfd[0]);
        free(args);
    }

    buf[*len] = '\0';
  
    return buf;
}

/********
* FUNCIÓN: void* file_parser(const char *filename, const char *mode, int* size_file)
* ARGS_IN: const char *filename - nombre del archivo; const char *mode - modo de lectura del archivo;
*          int *size_file - tamaño del fichero leido
* DESCRIPCIÓN: Dado una el nombre de un fichero y el modo de lectura, devolvemos el contenido del
*              fichero y guardamos el tamaño en size_file
* ARGS_OUT: void* - contenido del fichero (en size_file se guardará el tamaño del fichero)
********/
void* file_parser(const char *filename, const char *mode, int* size_file){
    FILE* f;
    char* ret = "";
    
    if(!filename || !mode) return NULL;
    if(strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0) return NULL;
    
    //Abrimos el file
    f = fopen(filename, mode);
    if (!f){
        syslog(LOG_ERR, "No file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);  //Vamos al final del fichero
    *size_file = ftell(f);  //Obtenemos el tamaño del file
    fseek(f, 0, SEEK_SET);  //Volvemos al inicio del fichero
    
    ret = (char*)malloc(*size_file*sizeof(char));
    if (!ret){
        
        fclose(f);
        return NULL;
    }

    //Leemos y guardamos el contenido del fichero
    if(fread(ret, 1, *size_file, f) == 0){
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    ret[*size_file-1] = '\0';

    return ret;
}

void free_params(char* params, int numparams){
    int i = 0;
    if (!params)
        return;

    for (i = 0; i < numparams; i++){
        if (!(params + i))
            free(params + i);
    }
    free(params);
}