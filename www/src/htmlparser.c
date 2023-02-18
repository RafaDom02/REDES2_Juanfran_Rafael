#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* html_parser(char* html){
    FILE* f;
    char* ret = "";
    int size_f;
    
    if (!html)
        return NULL;

    f = fopen(html, "r");
    if (!f){
        printf("No file");
        return NULL;
    }


    fseek(f, 0, SEEK_END);
    size_f = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
    ret = (char*)malloc(size_f*sizeof(char));
    
    if (!ret){
        
        fclose(f);
        return NULL;
    }

    fread(ret, 1, size_f, f);
    fclose(f);
    ret[size_f-1] = '\0';
    return ret;
}

long get_html_length(char* html){
    FILE *f;
    long size_f;

    if (!html)
        return NULL;

    f = fopen(html, "r");
    if (!f){
        printf("No file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_f = ftell(f);
    fseek(f, 0, SEEK_SET);

    fclose(f);

    return size_f; 
}
