#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "confuse.h"
#include "picohttpparser.h"
#include "http.h"
#include "errno.h"
#include "fileparser.h"
#include "types.h"
#include <netinet/in.h>
#include <netdb.h>
 
#define MAXEXT 6
#define MAXPATH 200
#define BUFLEN 10000


int connfd;

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


char* execute_script(char* path, int* len, char** params, int numparams){
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

STATUS GET(const char *path, const char* server_signature, int minor_version)
{
    int msglen, size_total, numparams = 0, i;
    char buf[BUFLEN], date[BUFLEN];
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

    syslog(LOG_INFO, "BEFORE\n");
    numparams = get_params(path, params);
    path = strtok(path, "?");
    syslog(LOG_INFO,"%s\n", path);

   time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);

    strftime(date, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_info);

    if (strcmp(INDEX1, path) == 0 || strcmp(INDEX2, path) == 0){
        syslog(LOG_INFO, "HTML Petition.\n");
        response = file_parser("media/html/index.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, minor_version, OK200, msglen,date, server_signature);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        free(response);
        return OK;
    }

    syslog(LOG_INFO, "Path: %s\n", path);
    extension = get_extension(path);
    syslog(LOG_INFO, "Extension: %s\n", extension);
    filename = get_file(path, extension);
    syslog(LOG_INFO, "Filename: %s\n", filename);
    response = file_parser(++path, "rb", &msglen);
   
    if(!response){
        syslog(LOG_INFO, "Response: No response\n");
        response = file_parser("media/html/error/e404.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, minor_version, ERROR404, msglen, date, server_signature);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        
        for (i = 0; i<numparams;i++){
        free(params[i]);
        }
        free(params);
        return OK;
    }
    else
        syslog(LOG_INFO, "Response: %s\n", (char*)response);
    if (strcmp(extension, HTML) == 0){
        syslog(LOG_INFO, "HTML Petition.\n");
        sprintf(buf, HTML_HEADER, minor_version, OK200, msglen, date, server_signature);
    }
    else if (strcmp(extension, JPG) == 0 || strcmp(extension, JPEG) == 0){
        syslog(LOG_INFO, "JPG/JPEG Petition.\n");
        sprintf(buf, JPG_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, PNG) == 0){
        syslog(LOG_INFO, "PNG Petition.\n");
        sprintf(buf, PNG_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, JS) == 0){
        syslog(LOG_INFO, "JS Petition.\n");
        sprintf(buf, JS_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, CSS) == 0){
        syslog(LOG_INFO, "CSS Petition.\n");
        sprintf(buf, CSS_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, SVG) == 0){
        syslog(LOG_INFO, "SVG Petition.\n");
        sprintf(buf, SVG_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, TXT) == 0){
        syslog(LOG_INFO, "TXT Petition.\n");
        sprintf(buf, TXT_HEADER, minor_version, OK200, --msglen,date, server_signature);
    }
    else if (strcmp(extension, GIF) == 0){
        syslog(LOG_INFO, "GIF Petition.\n");
        sprintf(buf, GIF_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, MPG) == 0 || strcmp(extension, MPEG) == 0){
        syslog(LOG_INFO, "MPG/MPEG Petition.\n");
        sprintf(buf, VIDEO_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, DOC) == 0){
        syslog(LOG_INFO, "DOC Petition.\n");
        sprintf(buf, DOC_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, DOCX) == 0){
        syslog(LOG_INFO, "DOCX Petition.\n");
        sprintf(buf, DOCX_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, PDF) == 0){
        syslog(LOG_INFO, "PDF Petition.\n");
        sprintf(buf, PDF_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, ICO) == 0){
        syslog(LOG_INFO, "ICO Petition.\n");
        sprintf(buf, ICO_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else if (strcmp(extension, PY) == 0 || strcmp(extension, PHP) == 0){
       
        syslog(LOG_INFO, "SCRIPT Petition.\n");
        response = execute_script(path, &msglen, params, numparams);
        sprintf(buf, TXT_HEADER, minor_version, OK200, msglen,date, server_signature);
    }
    else return ERROR;
    //buf es la cabecera y response es el binario, por que no vaaaa?
    
    size_total = strlen(buf) + msglen;
    syslog(LOG_INFO, "%d = %ld + %d\n", size_total, strlen(buf), msglen);
    
    total = malloc(size_total*sizeof(void));

    memcpy(total, buf, strlen(buf));
    memcpy(total + strlen(buf), response, msglen);

    send(connfd, total, size_total, 0);
    free(filename);
    free(extension);
    free(total);
    free(response);
    for (i = 0; i<numparams;i++){
        free(params[i]);
    }
    free(params);
    return OK;
}

STATUS POST(const char* path, const char* server_signature, int minor_version)
{
    
    char * msg, **params, buf[BUFLEN], date[BUFLEN], *total;
    int msglen = 0, numparams = 0, size_total = 0;
    
    int flag;

    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);

    strftime(date, 40, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
   
    params = (char**)malloc(BUFLEN*sizeof(char*));
    numparams = get_params(path, params);
    

    msg = execute_script(path, &msglen, params, numparams);
    sprintf(buf, TXT_HEADER, minor_version, OK200, msglen, date, server_signature);

    syslog(LOG_INFO, "msg: %s", msg);


    size_total = strlen(buf) + msglen;
    total = malloc(size_total*sizeof(void));

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

STATUS OPTIONS(const char*server_signature, int minor_version)
{
    char buf[BUFLEN];
    const char allowed_methods[] = "GET, POST, OPTIONS";
    int len;

    bzero(buf, BUFLEN);

    len = snprintf(buf, BUFLEN, OPTIONS_HEADER, minor_version, OK200, allowed_methods, server_signature);
    send(connfd, buf, len, 0);
    return OK;
}

int http(int fd, char* server_signature)
{
    char buf[BUFLEN];
    const char *method, *path;
    int pret, minor_version, msglen = 0, i;
    struct phr_header headers[MAXPATH];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    const void *response;

    connfd = fd;
    while (1)
    {
        /* read the request */
        while ((rret = read(connfd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR);       
        
        if (rret < 0)
            return -1;
        prevbuflen = buflen;
        buflen += rret;
       
        /* parse the request */
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
    path= strtok(path," ");

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
        if(POST(path, server_signature, minor_version) == ERROR)
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
    else{
        syslog(LOG_ERR, "UNKNOWN method petition.\n");
        response = file_parser("media/html/error/e400.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, ERROR404, msglen);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        free(response);
        return EXIT_FAILURE;
    }


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
