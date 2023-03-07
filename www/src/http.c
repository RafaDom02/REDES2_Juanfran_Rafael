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
#define JPG ".jpg"
#define JPEG ".jpeg"
#define TXT ".txt"
#define GIF ".gif"
#define MPG ".mpg"
#define MPEG ".mpeg"
#define DOC ".doc"
#define DOCX ".docx"
#define PDF ".pdf"

int sockt;

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

void *GET(const char *path)
{
    int msglen, size_total;
    char buf[BUFLEN];
    const void *extension, *filename, *response, *total;
    if (!path) return NULL;

    bzero(buf, BUFLEN);
    strcpy(buf, "HTTP/1.1\n");

    if (strcmp(INDEX1, path) == 0 || strcmp(INDEX2, path) == 0){
        response = file_parser("index.html", "r", &msglen);
       
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Lenght: %d\r\n\r\n", msglen);
        strcat(buf, (char*)response);
        send(sockt, buf, strlen(buf), 0);
        return NULL;
    }
    extension = get_extension(path);
    filename = get_file(path, extension);
    syslog(LOG_INFO, "Extension: %s\n", extension);
    syslog(LOG_INFO, "Filename: %s\n", filename);
    syslog(LOG_INFO, "Path: %s\n", path);
    response = file_parser(++path, "rb", &msglen);
   
    syslog(LOG_INFO, "-response->%s", response);
    if (strcmp(extension, JPG) == 0 || strcmp(extension, JPEG) == 0){
        syslog(LOG_INFO, "JPG/JPEG Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Lenght: %d\r\n\r\n", msglen);
    } 
    else if (strcmp(extension, TXT) == 0){
        syslog(LOG_INFO, "TXT Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else if (strcmp(extension, GIF) == 0){
        syslog(LOG_INFO, "GIF Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else if (strcmp(extension, MPG) == 0 || strcmp(extension, MPEG) == 0){
        syslog(LOG_INFO, "MPG/MPEG Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else if (strcmp(extension, DOC) == 0){
        syslog(LOG_INFO, "DOC Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: application/msword\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else if (strcmp(extension, DOCX) == 0){
        syslog(LOG_INFO, "DOCX Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: application/vnd.openxmlformats-officedocument."
                     "wordprocessingml.document\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else if (strcmp(extension, PDF) == 0){
        syslog(LOG_INFO, "PDF Petition\n");
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Lenght: %d\r\n\r\n", msglen);
    }
    else return NULL;
    //buf es la cabecera y response es el binario, por que no vaaaa?
    
    size_total = strlen(buf) + msglen;
    syslog(LOG_INFO, "%d = %d + %d", size_total, strlen(buf), msglen);
    
    total = malloc(size_total*sizeof(void));

    memcpy(total, buf, strlen(buf));
    memcpy(total + strlen(buf), response, msglen);

    send(sockt, total, size_total, 0);
    syslog(LOG_INFO, "AAAAAAAAAAAAAAAAAAAAAAAAAAA%x || %d\n", total, size_total);
    

    return NULL;
}

char *POST()
{
    return NULL;
}

char *OPTIONS()
{
    return NULL;
}

int http(int sock)
{
    char buf[10000];
    const char *method, *path;
    int pret, minor_version, msglen = 0, i;
    struct phr_header headers[100];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    char *response;

    sockt = sock;
    while (1)
    {
        /* read the request */
        while ((rret = read(sock, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR);       
        
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
        response = GET(path);
        }

    else if(method[0] == 'P'){
        syslog(LOG_INFO, "POST method petition.\n");
        response = POST();
        }

    else if(method[0] == 'O'){
        syslog(LOG_INFO, "OPTIONS method petition.\n");
        response = OPTIONS();
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
        syslog(LOG_INFO, "%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }

    return EXIT_SUCCESS;
}
