/** 
 * @file rventgas.c
 * @author Raphael Gruber (0828630)
 * @brief Outputs "Status ok" and returns with 0 in 6 out of 7 times
 * every 7 time it outputs "Pressure too high - immediate shutdown required and returns 1 
 * @date 02.05.2015
 */
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/** 
 * The name of the program
 */
static const char* modulname = "rventgas";

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
    int r = rand() % 7;

    if (r <= 5) {
        (void)printf("STATUS OK\n");
    } else {
        (void)printf("PRESSURE TOO HIGH - IMMEDIATE SHUTDOWN REQUIRED\n");
        error = EXIT_FAILURE;
    }

    return error;
}
