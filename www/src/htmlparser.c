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

    if(fread(ret, 1, size_f, f) == 0){
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    ret[size_f-1] = '\0';
    return ret;
}
