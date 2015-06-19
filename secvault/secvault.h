#ifndef _SECVAULT_H_
#define _SECVAULT_H_

#include <linux/ioctl.h>

struct dev_params {
    int id;
    char key[10];
    long size;
};

#define SECVAULT_IOC_MAGIC 's'

#define SECVAULT_IOC_CREATE     _IOW(SECVAULT_IOC_MAGIC, 1, struct dev_params*) 
#define SECVAULT_IOC_CHANGEKEY  _IOW(SECVAULT_IOC_MAGIC, 2, struct dev_params*)
#define SECVAULT_IOC_DELETE     _IOW(SECVAULT_IOC_MAGIC, 3, int)
#define SECVAULT_IOC_REMOVE     _IOW(SECVAULT_IOC_MAGIC, 4, int) 
#define SECVAULT_IOC_SIZE       _IOWR(SECVAULT_IOC_MAGIC, 5, int)

#define SECVAULT_IOC_MAXNR (5)

#define SECVAULT_KEY_LENGTH (10)
#define SECVAULT_MAJOR (231)
#define SECVAULT_MINOR (0)
#define SECVAULT_NR_DEVS (4)

#endif 
