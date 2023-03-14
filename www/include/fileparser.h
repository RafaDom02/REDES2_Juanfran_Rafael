#ifndef _FILEPARSER_H
#define _FILEPARSER_H


void* file_parser(const char *html, const char *mode, int* size_file);
const char* get_extension(const char *path);
const char* get_file(const char *path, const char *ext);
int get_params(char* extension, char** params);
char* execute_script(char* path, int* len, char** params, int numparams, char* form);
void free_params(char* params, int numparams);

#endif