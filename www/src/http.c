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
 
#define MAXEXT 6
#define MAXPATH 200
#define BUFLEN 10000
#define INDEX2 "/"
#define INDEX1 "/index.html"
#define HTML ".html"
#define JPG ".jpg"
#define JPEG ".jpeg"
#define TXT ".txt"
#define GIF ".gif"
#define MPG ".mpg"
#define MPEG ".mpeg"
#define DOC ".doc"
#define DOCX ".docx"
#define PDF ".pdf"
#define PY ".py"
#define PHP ".php"
#define ICO ".ico"
#define OPTIONS_HEADER "HTTP/1.1 200 OK\r\nAllow: %s\r\nContent-Length: 0\r\n\r\n"
#define HTML_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Lenght: %d\r\n\r\n"
#define IMAGE_HEADER "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Lenght: %d\r\n\r\n"
#define TXT_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Lenght: %d\r\n\r\n"
#define GIF_HEADER "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Lenght: %d\r\n\r\n"
#define VIDEO_HEADER "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Lenght: %d\r\n\r\n"
#define DOC_HEADER "HTTP/1.1 200 OK\r\nContent-Type: application/msword\r\nContent-Lenght: %d\r\n\r\n"
#define DOCX_HEADER "HTTP/1.1 200 OK\r\nContent-Type: application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\nContent-Lenght: %d\r\n\r\n"
#define PDF_HEADER "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Lenght: %d\r\n\r\n"
#define ICO_HEADER "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %d\r\n\r\n"

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

const char** get_params(char* extension){
    if(!extension) return NULL;

    char delim[] = "=";
    char **output;
    char *aux;
    char* token;
    int size = 0;
    

    aux = strtok(extension, delim);
    aux = strtok(extension, delim);

    token = strtok(aux, "+");

    while (aux != NULL){
        output = (char**)realloc(output, size+1);
        size++;
        output[size-1] = (char*)malloc(30*sizeof(char));
        strcpy(output[size-1],token);
        token = strtok(aux, NULL);

    }    
    
    return output;
}


STATUS GET(const char *path)
{
    int msglen, size_total;
    char buf[BUFLEN];
    const char *extension, *filename;
    const void *response, *total;
    if (!path) return ERROR;

    bzero(buf, BUFLEN);
    strcpy(buf, "HTTP/1.1\n");

    if (strcmp(INDEX1, path) == 0 || strcmp(INDEX2, path) == 0){
        syslog(LOG_INFO, "HTML Petition.\n");
        response = file_parser("media/html/index.html", "r", &msglen);
        sprintf(buf, HTML_HEADER, msglen);
        strcat(buf, (char*)response);
        send(connfd, buf, strlen(buf), 0);
        return ERROR;
    }
    extension = get_extension(path);
    filename = get_file(path, extension);
    response = file_parser(++path, "rb", &msglen);

    syslog(LOG_INFO, "Extension: %s\n", extension);
    syslog(LOG_INFO, "Filename: %s\n", filename);
    syslog(LOG_INFO, "Path: %s\n", path);
    syslog(LOG_INFO, "Response: %s\n", (char*)response);

    if (strcmp(extension, HTML) == 0){
        syslog(LOG_INFO, "JPG/JPEG Petition.\n");
        sprintf(buf, HTML_HEADER, msglen);
    }
    else if (strcmp(extension, JPG) == 0 || strcmp(extension, JPEG) == 0){
        syslog(LOG_INFO, "JPG/JPEG Petition.\n");
        sprintf(buf, IMAGE_HEADER, msglen);
    }
    else if (strcmp(extension, TXT) == 0){
        syslog(LOG_INFO, "TXT Petition.\n");
        sprintf(buf, TXT_HEADER, --msglen);
    }
    else if (strcmp(extension, GIF) == 0){
        syslog(LOG_INFO, "GIF Petition.\n");
        sprintf(buf, GIF_HEADER, msglen);
    }
    else if (strcmp(extension, MPG) == 0 || strcmp(extension, MPEG) == 0){
        syslog(LOG_INFO, "MPG/MPEG Petition.\n");
        sprintf(buf, VIDEO_HEADER, msglen);
    }
    else if (strcmp(extension, DOC) == 0){
        syslog(LOG_INFO, "DOC Petition.\n");
        sprintf(buf, DOC_HEADER, msglen);
    }
    else if (strcmp(extension, DOCX) == 0){
        syslog(LOG_INFO, "DOCX Petition.\n");
        sprintf(buf, DOCX_HEADER, msglen);
    }
    else if (strcmp(extension, PDF) == 0){
        syslog(LOG_INFO, "PDF Petition.\n");
        sprintf(buf, PDF_HEADER, msglen);
    }
    else if (strcmp(extension, ICO) == 0){
        syslog(LOG_INFO, "ICO Petition.\n");
        sprintf(buf, ICO_HEADER, msglen);
    }
    else return ERROR;
    //buf es la cabecera y response es el binario, por que no vaaaa?
    
    size_total = strlen(buf) + msglen;
    syslog(LOG_INFO, "%d = %ld + %d\n", size_total, strlen(buf), msglen);
    
    total = malloc(size_total*sizeof(void));

    memcpy(total, buf, strlen(buf));
    memcpy(total + strlen(buf), response, msglen);

    send(connfd, total, size_total, 0);
    return OK;
}

STATUS POST(const char* path)
{
    char * extension;
    int msglen = 0;

    extension = get_extension(path);
    FILE *fp;
    char out[BUFLEN];
    char buf[BUFLEN] = "";
    char comm[BUFLEN] = "/usr/bin/php .";
    char header[BUFLEN] = "";
    
    if (strcmp(extension, PY)==0){
        strcpy(comm, "/usr/bin/python3 .");
    }
        

    /* Open the command for reading. */
    
    strcat(comm, path);
    fp = popen(comm, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(out, sizeof(out), fp) != NULL) {
        strcat(buf, out);
    }

    msglen = strlen(buf);

    sprintf(header, TXT_HEADER, msglen);
    
    msglen += strlen(header);

    strcat(header, buf);
    send(connfd, header, msglen,0);

    pclose(fp);
    return OK;
}

STATUS OPTIONS()
{
    char buf[BUFLEN];
    const char *allowed_methods = "GET, POST, OPTIONS";
    int len;

    bzero(buf, BUFLEN);

    len = snprintf(buf, BUFLEN, OPTIONS_HEADER, allowed_methods);
    send(connfd, buf, len, 0);
    return OK;
}

int http(int fd)
{
    char buf[BUFLEN];
    const char *method, *path;
    int pret, minor_version, msglen = 0, i;
    struct phr_header headers[MAXPATH];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    char *response;

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
        if(GET(path) == ERROR)
        {
            syslog(LOG_ERR, "GET method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "GET method success.\n");
    }

    else if(method[0] == 'P'){
        syslog(LOG_INFO, "POST method petition.\n");
        if(POST(path) == ERROR)
        {
            syslog(LOG_ERR, "POST method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "POST method success.\n");
    }

    else if(method[0] == 'O'){
        syslog(LOG_INFO, "OPTIONS method petition.\n");
        if(OPTIONS() == ERROR)
        {
            syslog(LOG_ERR, "OPTIONS method failure.\n");
            return EXIT_FAILURE;
        }
        else syslog(LOG_INFO, "OPTIONS method success.\n");
    }

    else{
        syslog(LOG_ERR, "Not valid method.\n");
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
