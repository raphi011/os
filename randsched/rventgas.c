#include <stdio.h>

int main(int argc, char* argv[]) {
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
