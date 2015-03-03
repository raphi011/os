#include <stdio.h>
#include <stdlib.h>

#define MAX_CHARS (15)

char * modulename;

int main(int argc, char** argv) {
    modulename = argv[0];

    char * input = malloc(MAX_CHARS);
    
    while (fgets(input, MAX_CHARS, stdin)) {

        int num1,num2;
        char operation;

        int returnValue = sscanf(input, "%d %d %c", &num1, &num2, &operation);
        
        if (returnValue == 3) {
            pid_t pid;

            switch (pid = fork()) {
                case -1: break; // error
                case 0; break; // childprocess
                default: break; // parentprocess

                /* int result; 
                switch (operation) {
                    case '*': result = num1*num2; break;
                    case '+': result = num1+num2; break;
                    case '-': result = num1-num2; break;
                    case '/': result = num1/num2; break;
                }*/

            }
    printf("%d\n", result);
        } else if (returnValue == EOF) {
            
        } else {

        }
    }
}

