#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 100 // Maximum number of variables to split

int split_variables(char* str, char** variables) {
    char* aux = strtok(str, "=");
    aux = strtok(NULL, "=");
    
    
    
    int count = 0;
    char* token = strtok(aux, "+"); // Split the string by plus signs
    while (token != NULL && count < MAX_VARIABLES) {
        variables[count++] = token; // Store the variable in the array
        token = strtok(NULL, "+"); // Move to the next variable
    }
    return count;
}

int main() {
    char str[] = "variables=variable1+variable2+variable3+v4";
    char* variables[MAX_VARIABLES];
    int count = split_variables(str, variables);
    printf("%d variables found:\n", count);
    for (int i = 0; i < count; i++) {
        printf("%d: %s\n", i + 1, variables[i]);
    }
    return 0;
}