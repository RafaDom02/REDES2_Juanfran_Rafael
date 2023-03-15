#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include "http.h"
#include "confuse.h"
#include "picohttpparser.h"
#include "errno.h"
#include "fileparser.h"
#include "types.h"
#include <netinet/in.h>
#include <netdb.h>
 
#define MAXPATH 200


int connfd;

/********
* FUNCIÓN: STATUS GET(const char *path, const char* server_signature, int minor_version)
* ARGS_IN: const char* path - path del archivo pedido; const char* server_signature - nombre del servidor;
*          int minor_version - version del HTTP/1.x
* DESCRIPCIÓN: Se encarga de ejecutar el codigo para el correcto funcionamiento del método GET
* ARGS_OUT: STATUS - OK o ERROR
********/
STATUS GET(const char *path, const char* server_signature, int minor_version){
    int msglen, size_total, numparams = 0, i, buflen;
    char buf[BUFLEN], date[BUFLEN], lastmodified[BUFLEN];
    const char *extension, *filename;
    char ** params;
    const void *response, *total;
    if (!path) return ERROR;

    bzero(buf, BUFLEN);
    strcpy(buf, "HTTP/1.1\n");

    params = (char**)malloc(100*sizeof(char*));
    if (!params){
        return EXIT_FAILURE;
    }

    numparams = get_params(path, params);
    path = strtok(path, "?");
    syslog(LOG_INFO,"%s\n", path);

    //Obtenemos la fecha actual y la fecha de ultima modificación del fichero deseado
    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now), *tm_lm;
    struct stat attr;

    strftime(date, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_info);

    stat(path, &attr);
    tm_lm = gmtime(&(attr.st_mtime));
    strftime(lastmodified, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_lm);

    //En caso que el path sea ip:port/ o ip:port/index.html, envia el fichero index.html
    if (strcmp(INDEX1, path) == 0 || strcmp(INDEX2, path) == 0){
        syslog(LOG_INFO, "HTML Petition.\n");
        response = file_parser("media/html/index.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, minor_version, OK200, msglen,date,lastmodified, server_signature);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        free(response);
        return OK;
    }

    //Obtienemos la extensión del fichero, el nombre del fichero y el contenido del fichero
    syslog(LOG_INFO, "Path: %s\n", path);
    extension = get_extension(path);
    syslog(LOG_INFO, "Extension: %s\n", extension);
    filename = get_file(path, extension);
    syslog(LOG_INFO, "Filename: %s\n", filename);
    response = file_parser(++path, "rb", &msglen);
   
    //En caso de que no exista el fichero enviamos el error 404
    if(!response){
        syslog(LOG_INFO, "Response: No response\n");
        response = file_parser("media/html/error/e404.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, minor_version, ERROR404, msglen, date, lastmodified, server_signature);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        
        for (i = 0; i<numparams;i++){
        free(params[i]);
        }
        free(params);
        if (response != NULL)
            free(response);
        
        return OK;
    }
    else
        syslog(LOG_INFO, "Response: %s\n", (char*)response);
    
    //En caso contrario, asociamos una cabecera dependiendo del tipo de extension
    if (strcmp(extension, HTML) == 0){
        syslog(LOG_INFO, "HTML Petition.\n");
        sprintf(buf, HTML_HEADER, minor_version, OK200, msglen, date, lastmodified, server_signature);
    }
    else if (strcmp(extension, JPG) == 0 || strcmp(extension, JPEG) == 0){
        syslog(LOG_INFO, "JPG/JPEG Petition.\n");
        sprintf(buf, JPG_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, PNG) == 0){
        syslog(LOG_INFO, "PNG Petition.\n");
        sprintf(buf, PNG_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, PY) == 0 || strcmp(extension, PHP) == 0){
       
        syslog(LOG_INFO, "SCRIPT Petition.\n");
        free(response);
        response = execute_script(path, &msglen, params, numparams, NULL);
        
        sprintf(buf, TXT_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
        
    
    }
    else if (strcmp(extension, JS) == 0){
        syslog(LOG_INFO, "JS Petition.\n");
        sprintf(buf, JS_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, CSS) == 0){
        syslog(LOG_INFO, "CSS Petition.\n");
        sprintf(buf, CSS_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, SVG) == 0){
        syslog(LOG_INFO, "SVG Petition.\n");
        sprintf(buf, SVG_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, TXT) == 0){
        syslog(LOG_INFO, "TXT Petition.\n");
        sprintf(buf, TXT_HEADER, minor_version, OK200, --msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, GIF) == 0){
        syslog(LOG_INFO, "GIF Petition.\n");
        sprintf(buf, GIF_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, MPG) == 0 || strcmp(extension, MPEG) == 0){
        syslog(LOG_INFO, "MPG/MPEG Petition.\n");
        sprintf(buf, VIDEO_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, DOC) == 0 || strcmp(extension, DOCX) == 0){
        syslog(LOG_INFO, "DOC Petition.\n");
        sprintf(buf, DOC_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, PDF) == 0){
        syslog(LOG_INFO, "PDF Petition.\n");
        sprintf(buf, PDF_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    else if (strcmp(extension, ICO) == 0){
        syslog(LOG_INFO, "ICO Petition.\n");
        printf("NOS LLEGA UNA PETICION ICO.");
        sprintf(buf, ICO_HEADER, minor_version, OK200, msglen,date, lastmodified, server_signature);
    }
    
    else return ERROR;
    
    size_total = strlen(buf) + msglen;
    syslog(LOG_INFO, "%d = %ld + %d\n", size_total, strlen(buf), msglen);
    
    total = malloc(size_total*sizeof(void));

    //Enviamos la cabecera junto al contenido del fichero
    memcpy(total, buf, strlen(buf));
    memcpy(total + strlen(buf), response, msglen);

    send(connfd, total, size_total, 0);

    free(total);
    if (response != NULL)
        free(response);
    
    free_params(params, numparams);
    
    return OK;
}

/********
* FUNCIÓN: STATUS POST(const char* path, const char* server_signature, int minor_version, char* form)
* ARGS_IN: const char* path - path del archivo pedido; const char* server_signature - nombre del servidor;
*          int minor_version - version del HTTP/1.x; char* form - formulario de la pagina POST
* DESCRIPCIÓN: Se encarga de ejecutar el codigo para el correcto funcionamiento del método POST
* ARGS_OUT: STATUS - OK o ERROR
********/
STATUS POST(const char* path, const char* server_signature, int minor_version, char* form){
    
    char * msg = NULL, **params = NULL, buf[BUFLEN] = "", date[BUFLEN] = "", lastmodified[BUFLEN] = "", *total = NULL;
    int msglen = 0, numparams = 0, size_total = 0;
    
    int flag = 0;

    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now), *tm_lm = NULL;
    struct stat attr;

    
    

    //Obtenemos la fecha actual y la fecha de ultima modificación del fichero deseado
    strftime(date, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_info);

    stat(path+1, &attr);
    tm_lm = gmtime(&(attr.st_mtime));
    if (tm_lm != NULL){
    strftime(lastmodified, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_lm);
    }
    
   
    params = (char**)malloc(BUFLEN*sizeof(char*));
    numparams = get_params(path, params);
    

    msg = execute_script(path, &msglen, params, numparams, form);
    sprintf(buf, TXT_HEADER, minor_version, OK200, msglen, date, lastmodified, server_signature);

    if (!msg){
        exit(1);
    }
    
    syslog(LOG_INFO, "msg: %s", msg);

    size_total = strlen(buf) + msglen;
    total = (char*)malloc(size_total*sizeof(void));
    if (!total){
        exit(1);
    }
    strcpy(total, "");
    
    memcpy(total, buf, strlen(buf));
    memcpy(total + strlen(buf), msg, msglen);
    
    flag = send(connfd, total, size_total,0);

    for (int i = 0; i < numparams; i++)
        free(params[i]);
   
    free(params);
    free(msg);
    free(total);
    return OK;
}

/********
* FUNCIÓN: STATUS OPTIONS(const char*server_signature, int minor_version)
* ARGS_IN: const char *server_signature - nombre del servidor; int minor_version - version del HTTP/1.x
* DESCRIPCIÓN: Envia por la conexión los distintos métodos disponibles
* ARGS_OUT: STATUS - OK o ERROR
********/
STATUS OPTIONS(const char*server_signature, int minor_version)
{
    char buf[BUFLEN];
    const char allowed_methods[] = "GET, POST, OPTIONS";
    int len;

    bzero(buf, BUFLEN);

    //Enviamos la cabecera junto a los métodos permitidos en el servidor
    len = snprintf(buf, BUFLEN, OPTIONS_HEADER, minor_version, OK200, allowed_methods, server_signature);
    send(connfd, buf, len, 0);
    return OK;
}

/********
* FUNCIÓN: int http(int fd, char* server_signature)
* ARGS_IN: int fd - conexion abierta por la función accept; char* server_signature - nombre del servidor
* DESCRIPCIÓN: Obtiene la petición de la conexión y llama a la función GET, POST u OPTIONS dependiendo del método pedido
* ARGS_OUT: int - EXIT_SUCCESS o EXIT_FAILURE
********/
int http(int fd, char* server_signature)
{
    char buf[BUFLEN];
    const char *method, *path;
    int pret, minor_version, msglen = 0, i;
    struct phr_header headers[MAXPATH];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    const void *response;
    char *content_type = NULL;
    int content_length = 0;
    char post_form[BUFLEN] = "";

    connfd = fd;
    while (1)
    {
        //Leemos la conexión hasta que llegue una petición
        while ((rret = read(connfd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR);       
        
        if (rret < 0)
            return -1;
        prevbuflen = buflen;
        buflen += rret;
       
        //Llamamos a phr_parse_request para obtener los datos de la conexion
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                                 &minor_version, headers, &num_headers, prevbuflen);
        if(pret >= 0) break;

        if(pret == -1){
            syslog(LOG_ERR, "Error with phr_parse_request.\n");
            return EXIT_FAILURE;
        }
        if(pret == -2){
            syslog(LOG_WARNING, "phr_parse_request request is partial.\n");
        }
        
    }
    //Dividimos del path para solo obtener el nombre del fichero deseado
    path= strtok(path," ");

    //Dependiendo del método deseado, llamamos a una fución distinta
    if (method[0] == 'G'){
        syslog(LOG_INFO, "GET method petition.\n");
        if(GET(path, server_signature, minor_version) == ERROR)
        {
            syslog(LOG_ERR, "GET method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "GET method success.\n");
    }
    else if(method[0] == 'P'){
        syslog(LOG_INFO, "POST method petition.\n");

        
        strcpy(post_form, headers[num_headers-1].name);
        strtok(post_form, "\r\n");
        strcpy(post_form, strtok(NULL, "\r\n"));
        
        printf("%s\n", post_form);

        for (int i = 0; i < num_headers; i++) {
            
            if (strcasecmp(strtok(headers[i].name,":"), "Content-Type") == 0) {
            content_type = strtok(headers[i].value, "\n");
            
            
            
            } else if (strcasecmp(headers[i].name, "Content-Length") == 0) {
            content_length = atoi(strtok(headers[i].value, "\n"));
            } 
        }
        
        if(content_type != NULL)    
            if (strcasecmp(content_type, "application/x-www-form-urlencoded\r") != 0){
           
            return -1;
        }
        
        if(POST(path, server_signature, minor_version, post_form) == ERROR)
        {
            syslog(LOG_ERR, "POST method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "POST method success.\n");
    }
    else if(method[0] == 'O'){
        syslog(LOG_INFO, "OPTIONS method petition.\n");
        if(OPTIONS(server_signature, minor_version) == ERROR)
        {
            syslog(LOG_ERR, "OPTIONS method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "OPTIONS method success.\n");
    }
    else{ //En caso de ser una petición desconocida, ejecutamos el error 400
        syslog(LOG_ERR, "UNKNOWN method petition.\n");
        time_t now = time(NULL);
        struct tm* tm_info = gmtime(&now), *tm_lm;
        struct stat attr;
        char* date, *lastmodified;

        //Obtenemos la fecha actual y la fecha de ultima modificación del fichero deseado
        strftime(date, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
        stat(path+1, &attr);
        tm_lm = gmtime(&(attr.st_mtime));
        strftime(lastmodified, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_lm);

        response = file_parser("media/html/error/e400.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, minor_version, ERROR404, msglen, date, lastmodified, server_signature);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        free(response);
        return EXIT_FAILURE;
    }

    //Imprimimos en el log los datos de la petición
    syslog(LOG_INFO, "Minor version: %d\n", minor_version);
    syslog(LOG_INFO, "request is %d bytes long\n", pret);
    syslog(LOG_INFO, "method is %.*s\n", (int)method_len, method);
    syslog(LOG_INFO, "path is %.*s\n", (int)path_len, path);
    syslog(LOG_INFO, "HTTP version is 1.%d\n", minor_version);
    syslog(LOG_INFO, "headers:\n");
    for (i = 0; i != num_headers; ++i)
    {
        syslog(LOG_INFO, "\t%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }

    return EXIT_SUCCESS;
}
