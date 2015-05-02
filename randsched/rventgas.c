#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static const char* modulname = "rventgas";

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
        printf("STATUS OK\n");
    } else {
        printf("PRESSURE TOO HIGH - IMMEDIATE SHUTDOWN REQUIRED\n");
        error = 1;
    }

    return error;
}
