/** 
 * @file rshutdown.c
 * @author Raphael Gruber (0828630)
 * @brief Outputs "SHUTDOWN COMPLETED" and returns with 0 in 1 out of 3 times
 * else it outputs "KaBOOM!" and returns 1 
 * @date 02.05.2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/** 
 * The name of the program
 */
static const char* modulname = "rshutdown";

/**
 * main
 * @brief The main entry point of the program.
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @return The exit code of the program. EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char *argv[]) {
    
    if (argc > 0) {
        modulname = argv[0]; 
    }

    if (argc > 1) {
        (void)fprintf(stderr, "usage: %s\n", modulname);
        exit(EXIT_FAILURE);
    }

    int error = 0;

    srand(time(NULL));
    int r = rand() % 3;

    if (r == 0) {
        (void)printf("SHUTDOWN COMPLETED\n");
    } else {
        (void)printf("KaBOOM!\n");
        error = 1;
    }

    return error;
}
