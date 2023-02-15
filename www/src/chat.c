#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "chat.h"

#define SIZE_MSG 100


int chat(int fd){
    ssize_t size;
    char* buffer = (char*)malloc(SIZE_MSG*sizeof(char));
    char* response = (char*)malloc(SIZE_MSG*sizeof(char));
    if (!buffer){
        perror("Error allocating the buffer.\n");
        return 1;
    }

    while(1){
        size = read(fd, buffer, SIZE_MSG);
        buffer[size] = '\0';
        
        if (strcmp(buffer, "exit") == 0){
            return 0;
        }
        else if (strcmp(buffer, "\n") != 0){
            printf("Respuesta %s.\n", buffer);

            scanf("%s", response);


            write(fd, response, sizeof(response));
        }
    }

    free(response);
    free(buffer);
}