#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char* modulname = "rshutdown";

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
        printf("SHUTDOWN COMPLETED\n");
    } else {
        printf("KaBOOM!\n");
        error = 1;
    }

    return error;
}
