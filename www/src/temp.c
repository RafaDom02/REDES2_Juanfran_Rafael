#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "picohttpparser.h"

#define MAX_REQUEST_SIZE 4096
#define MAX_RESPONSE_SIZE 4096

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    char request[MAX_REQUEST_SIZE], response[MAX_RESPONSE_SIZE];
    int bytes_received, bytes_sent;
    int num_headers, method, minor_version, status;
    const char *msg, *method_str, *path;
    struct phr_header headers[100];

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // bind to port 8080
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // listen for incoming connections
    listen(sockfd, 5);
    printf("Server listening on port 8080...\n");

    while (1) {
        // accept incoming connection
        cli_len = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        // read request
        bytes_received = recv(newsockfd, request, MAX_REQUEST_SIZE, 0);
        if (bytes_received < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        // parse request
        num_headers = sizeof(headers) / sizeof(headers[0]);
        if (phr_parse_request(request, bytes_received, &method_str, &method, &path, &minor_version, headers, &num_headers, 0) != bytes_received) {
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            bytes_sent = send(newsockfd, response, strlen(response), 0);
            if (bytes_sent < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            close(newsockfd);
            continue;
        }

        printf("Received request:\n%s", request);
        printf("Method: %.*s\n", (int)(method_str - request), method_str);
        printf("Path: %.*s\n", (int)(path - request), path);

        // create response
        if (method == HTTP_GET) {
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!");
        } else {
            sprintf(response, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
        }

        // send response
        bytes_sent = send(newsockfd, response, strlen(response), 0);
        if (bytes_sent < 0) {
            perror("ERROR writing to socket");
            exit(
