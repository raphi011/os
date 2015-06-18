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

#define KEY_LENGTH (10)

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

    int secvault_size, secvault_id;
    char c;
    int create = 0, key = 0, clear = 0, remove = 0;
    
    while ((c = getopt (argc, argv, "kedc:")) != -1) {
        switch (c) {
            case 'k':
                key = 1;
                break;
            case 'e':
                clear = 1;
                break;
            case 'd':
                remove = 1;
                break;
            case 'c':
                create = 1;
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

    if ((argc - 1) != optind) {
        usage(); 
    } else {
        if (!parse_int(argv[optind], &secvault_id)) {
            usage();
        }
    }

    int par_cnt = (create + key + clear + remove);

    if (par_cnt > 1) {
        usage(); 
    }

    devfd = open("/dev/sv_ctl", O_RDWR); 

    if (devfd == -1) {
        bail_out(EXIT_FAILURE, "couldn't open /dev/sv_ctl");
    }

    int ret = 0;
    struct dev_params params = {0};
    params.id = secvault_id;

    if (create == 1 || key == 1) {
        char *buf = malloc(KEY_LENGTH + 1); 
        (void)printf("Please enter a new key with a maximum of %d chars:\n", KEY_LENGTH);
        if (fgets(buf, KEY_LENGTH + 1, stdin) == NULL) {
            bail_out(EXIT_FAILURE, "missing key\n");
        } else {
            (void)memcpy(params.key, buf, strlen(buf)); 
        }
    }
    if (par_cnt == 0) {
        // output size of device
        ret = ioctl(devfd, SECVAULT_IOC_SIZE, &secvault_id); 
        if (ret > 0) {
            (void)printf("size of secvault %d: %d\n", secvault_id, ret);
        } else {
            bail_out(EXIT_FAILURE, "ioctl size error: %s\n", strerror(errno));
        }
    } else if (create == 1) {
        params.size = secvault_size;
        ret = ioctl(devfd, SECVAULT_IOC_CREATE, &params); 
        (void)printf("secvault with id %d created\n", secvault_id);
        if (ret == -1) {
            bail_out(EXIT_FAILURE, "ioctl create error: %s\n", strerror(errno));
        }
    } else if (key == 1) {
        ret = ioctl(devfd, SECVAULT_IOC_CHANGEKEY, &secvault_id); 
        if (ret == -1) {
            bail_out(EXIT_FAILURE, "ioctl changekey error: %s\n", strerror(errno));
        }
    } else if (clear == 1) {
        ret = ioctl(devfd, SECVAULT_IOC_DELETE, &secvault_id); 
        if (ret == -1) {
            bail_out(EXIT_FAILURE, "ioctl delete error: %s\n", strerror(errno));
        }
    } else { // remove == 1
        ret = ioctl(devfd, SECVAULT_IOC_REMOVE, &secvault_id); 
        if (ret == -1) {
            bail_out(EXIT_FAILURE, "ioctl create error: %s\n", strerror(errno));
        }
    }

    free_resources();

    return EXIT_SUCCESS;
}

