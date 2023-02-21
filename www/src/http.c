#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "chat.h"
#include "confuse.h"
#include "picohttpparser.h"
#include "http.h"
#include "errno.h"
#include "htmlparser.h"

#define INDEX1 "/index.html"
#define INDEX2 "/"


char * GET(char* path){
    if (!path)
        return -1;

    

    if (strcmp(INDEX1, path) == 0 || strcmp(INDEX2, path) == 0){
        return html_parser("index.html");
    }
    
}


char* POST(){

}

char* OPTIONS(){

}


int http(int sock){
    char buf[4096], *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;
    char method_char;
    char* response;

    while (1) {
    /* read the request */
    while ((rret = read(sock, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR)
        ;
    if (rret <= 0)
        return -1;
    prevbuflen = buflen;
    buflen += rret;

    
    if (buf)
        printf("%s\n",buf);
    /* parse the request */
    num_headers = sizeof(headers) / sizeof(headers[0]);
    pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                             &minor_version, headers, &num_headers, prevbuflen,);
    printf("pret:%d\n",pret);
    if (pret >= 0){
       
    
    
        path[path_len] = '\0';


        switch (method[0])
        {
        printf("%s", path);
        case 'G':
            response = GET(path);
            break;

        case 'P':
            response = POST();
            break;

        case 'O':
            response = OPTIONS();
            break;
        
        default:
            
        }

        //response = phr_parse      _response(response, sizeof(response), minor_version, )

        pret = phr_parse_response(response, strlen(response),&minor_version,NULL,&response, strlen(response),headers, &num_headers,)

        int a = write(sock, response, 5374);
        printf("%d\n", a);

        if (minor_version == 0)
            break;
    }
    
        
    /* request is incomplete, continue the loop */
    
       
    
    }
    int i = 0;

    printf("request is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    for (i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
            (int)headers[i].value_len, headers[i].value);
    }

}

