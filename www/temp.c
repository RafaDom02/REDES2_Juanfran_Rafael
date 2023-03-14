#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

int main(void) {
    struct tm *clock;
    struct stat attr;
    char file[] = "media/html/index.html";

    stat(file, &attr);
    clock = gmtime(&(attr.st_mtime));

    printf("%s\n", asctime(clock));
    return 0;
}