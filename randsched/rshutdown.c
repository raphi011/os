#include <stdio.h>

int main(int argc, char* argv[]) {
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
