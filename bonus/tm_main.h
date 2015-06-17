#ifndef _SECVAULT_H_
#define _SECVAULT_H_

#include <linux/ioctl.h>

#define SECVAULT_IOC_MAGIC ('s')

#define SECVAULT_IOC_CREATE     _IOW(SECVAULT_IOC_MAGIC, 1, int) 
#define SECVAULT_IOC_CHANGEKEY  _IOW(SECVAULT_IOC_MAGIC, 2, char*)
#define SECVAULT_IOC_DELETE     _IOW(SECVAULT_IOC_MAGIC, 3, int)
#define SECVAULT_IOC_SIZE       _IOR(SECVAULT_IOC_MAGIC, 4, int)
#define SECVAULT_IOC_REMOVE     _IO(SECVAULT_IOC_MAGIC, 5) 

#endif 
