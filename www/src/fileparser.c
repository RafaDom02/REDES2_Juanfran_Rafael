#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#define PATH "/www/"

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
