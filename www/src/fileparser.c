#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <strings.h>
#include "http.h"
#include "types.h"

#define PATH "/www/"

/**
 * @brief Get the extension of a file
 * 
 * @param path string with the path
 * @return extension
 */
const char* get_extension(const char *path){
    char *s;
    if(!path) return NULL;

    s = &(path[strlen(path)-1]);

    for(; path != s && s[0] != '.'; s--);    //Comprobamos que que file no es mayor al path dado o que s[0] sea una .

    if(s[0] == '.') return s;                //Si ha acabado el for-loop con el punto, devuelve la extension
    return NULL;
}

/**
 * @brief Get the file name from a path
 * 
 * @param path string with the path
 * @param ext string with the extension
 * @return file name
 */
const char* get_file(const char *path, const char *ext){
    char *s = strstr(path, ext);
    if(!s) return NULL;

    for(; path != s && s[0] != '/'; s--);   //Comprobamos que que file no es mayor al path dado o que s[0] sea una /

    if(s[0] == '/') return ++s;             //Eliminamos la barra del directorio si ha acabado el for-loop por la barra
    return NULL;
}

int get_params(char* extension, char** params){
    if(!extension) return NULL;
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

char* execute_script2(char* path, int* len, char** params, int numparams){
    char * extension, *aux;
    int msglen = 0, i;

    aux = strtok(path, "?");
    extension = get_extension(aux);
    FILE *fp;
    char out[BUFLEN];
    char buf[BUFLEN] = "";
    char comm[BUFLEN] = "/usr/bin/php ./";
    char header[BUFLEN] = "";
    char *output;
    
    if (strcmp(extension, PY)==0){
        strcpy(comm, "/usr/bin/python3 ./");
    }

    output = (char*)malloc(BUFLEN*sizeof(char));
    if (!output){
        return NULL;
    }
        

    /* Open the command for reading. */
    if (numparams > 0){
        path = strtok(path, "?");
    }
    strcat(comm, path);
    printf("numparams: %d\n", numparams);
    for (i = 0; i<numparams; i++){
        
        strcat(comm, " ");
        strcat(comm, params[i]);
    }

    
    printf("COMM: %s\n", comm);

    fp = popen(comm, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return NULL;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(out, sizeof(out), fp) != NULL) {
        strcat(buf, out);
    }

    msglen = strlen(buf);


    strcat(header, buf);

    printf("HEAD: %s\n", header);

    pclose(fp);

    *len = msglen;
    strcpy(output, header);
    return output;
}

char* execute_script(char* path, int* len, char** params, int numparams, char* form){
    char * extension, *aux;
    int msglen = 0, i;
    aux = strtok(path, "?");
    extension = get_extension(aux);

    int pipefd[2];
    char *buf;
    char aux2[BUFLEN];
    char comm[BUFLEN] = "/usr/bin/php";
    char name[BUFLEN] = "php";
    char header[BUFLEN] = "";
    char *output, **args;

    buf = (char*)malloc(BUFLEN*sizeof(char));
    
    if (strcmp(extension, PY)==0){
        strcpy(comm, "/usr/bin/python3");
        strcpy(name,"python3");
    }

    // create a pipe for communication
    if (pipe(pipefd) == -1) {
        perror("pipe");
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



   
    
    printf("name: %s\n", name);

    for (i = 0; i< numparams +2; i++){
        printf("AAA:%s\n", args[i]);
    }

    // execute the Python script
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // child process - replace it with the Python script
        close(pipefd[1]);  // close the write end of the pipe
        dup2(pipefd[0], STDIN_FILENO);  // replace stdin with the read end of the pipe
        execv(comm, args);
        perror("execl");
        exit(1);
    } else {
        // parent process - send input to the script and read output from the script
        close(pipefd[0]);  // close the read end of the pipe
        write(pipefd[1], form, strlen(form));  // send input to the script
        *len = read(pipefd[0], buf, 100);  // read output from the script
       
        close(pipefd[1]);
        free(args);
    }

    return buf;


}
void* file_parser(const char *html, const char *mode, int* size_file){
    FILE* f;
    char* ret = "";
    
    
    if (!html)
        return NULL;
    f = fopen(html, "r");
    if (!f){
        syslog(LOG_ERR, "No file");
        return NULL;
    }


    fseek(f, 0, SEEK_END);
    *size_file = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
    ret = (char*)malloc(*size_file*sizeof(char));
    
    if (!ret){
        
        fclose(f);
        return NULL;
    }

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
        if (params[i] == NULL)
            free(params[i]);
    }
    free(params);
}