#include <stdio.h>
#include <stdlib.h>

static const char *modulname = "svctl";

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-c <size>|-k|-e|-d] <secvault_id>\n", modulname);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc > 0) {
        modulname = argv[0];
    }

    usage();
}

