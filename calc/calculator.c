#include <limits.h> 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHARS (15)

char * modulename;

int main(int argc, char** argv) {
    modulename = argv[0];

    int fildes1[2], fildes2[2];
    if (pipe(fildes1) || pipe(fildes2))
    {
        // error 
    }
    pid_t pid;
    char * input;

    switch (pid = fork()) {
        case -1: 
            break; // error
        case 0:
            input = malloc(MAX_CHARS);
            close(fildes1[1]);
            //FILE *fp = fdopen(fildes1[0], "r");

            while (read(fildes1[0], input, MAX_CHARS)) {
               printf("%S\n", input); 
            }
            
            exit(EXIT_SUCCESS); 
        default:  // parentprocess
            close(fildes1[0]);
            //close(fileno(stdin));
            dup2(fileno(stdin), fildes1[1]);
            //close(fildes1[1]);
            wait();
            break;
    }
      /*  int num1,num2;
        char operation;

        int returnValue = sscanf(input, "%d %d %c", &num1, &num2, &operation);
        
        if (returnValue == 3) {
            pid_t pid;


                 int result; 
                switch (operation) {
                    case '*': result = num1*num2; break;
                    case '+': result = num1+num2; break;
                    case '-': result = num1-num2; break;
                    case '/': result = num1/num2; break;
                }

            }
    printf("%d\n", result);
        } else if (returnValue == EOF) {
            
        } else {

        }
    }*/
}

