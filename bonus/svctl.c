#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "tm_main.h"

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

static const char *modulname = "svctl";

int devfd; 

static void usage() {
    (void)fprintf(stderr, "USAGE: %s [-c <size>|-k|-e|-d] <secvault_id>\n", modulname);
    exit(EXIT_FAILURE);
}

static void free_resources(void)
{
    /* clean up resources */
    if (devfd >= 0) {
        (void)close(devfd);
    }
}

long parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    if (errno == ERANGE) {
        return 0;
    }

    return (*endptr == '\0'); 
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
    if (argc > 0) {
        modulname = argv[0];
    }

    int secvault_size;
    char c;
    
    while ((c = getopt (argc, argv, "kedc:")) != -1) {
        switch (c) {
            case 'k':
                // todo
                break;
            case 'e':
                // todo
                break;
            case 'd':
                // todo
                break;
            case 'c':
                if (!parse_int(optarg, &secvault_size)) {
                    usage();   
                } 
                break;
            case '?':
                usage();   
            default: 
                assert(0);
        }
    }

    /* if (argc != optind) {
        usage(); 
    } */

    devfd = open("/dev/sv_ctl", O_RDWR); 

    if (devfd == -1) {
        bail_out(EXIT_FAILURE, "couldn't open /dev/sv_ctl");
    }

    int vaultno = 1;

    int ret = ioctl(devfd, SECVAULT_IOC_CREATE, &vaultno); 

    if (ret == -1) {
        bail_out(EXIT_FAILURE, "ioctl error: %s\n", strerror(errno));
    }

    free_resources();

    return EXIT_SUCCESS;
}

