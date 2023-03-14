#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <strings.h>



int main(){
     int pipefd[2];
    char buf[100];
    char aux2[100];
    char comm[100] = "/usr/bin/php ./";
    char name[100] = "php";
    char header[100] = "";
    char form[100] = "a a a\n";
   
    
    
    strcpy(comm,"/usr/bin/python3");
    char *args[] = {"python3",".//media/scripts/test.py", NULL};
    
    
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // child process - replace it with the Python script
        close(pipefd[1]);  // close the write end of the pipe
        dup2(pipefd[0], STDIN_FILENO);  // replace stdin with the read end of the pipe
        execv(comm, args);
        perror("execv");
        exit(1);
    } else {
        // parent process - send input to the script and read output from the script
        close(pipefd[0]);  // close the read end of the pipe
        write(pipefd[1], form, strlen(form));  // send input to the script
        read(pipefd[0], buf, 100);  // read output from the script
       
        close(pipefd[1]);
       
    }
}