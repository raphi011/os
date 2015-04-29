#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
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
