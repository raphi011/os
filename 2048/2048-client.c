#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

#include "helper.h"


static const char* modulname;

static void usage() {
    (void)fprintf(stderr, "USAGE: 2048-client [-n | -i <id>]\n\t-n:\tStart a new game\n\t-i:\tConnect to existing game with the given id",modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void) {

}

static void bail_out(int eval, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", modulname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(eval);
}

int main(int argc, char* argv[]) {
    modulname = argv[0];
    int c; 
    bool new_game = false;
    int game_id; 

    while ((c = getopt (argc, argv, "ni:")) != -1) {
        switch (c) {
            case 'i':
                if (parse_int(optarg, &game_id)) {
                    usage();   
                } 
                break;
            case 'n':
                new_game = true;
                break;
            case '?':
                usage();   
            default: 
                assert(0);
        }
    }

    if (argc != optind) {
        usage(); 
    }
}
