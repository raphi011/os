#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "tm_main.h"

#define SECVAULT_MAJOR (231) // use 231 as major device number!
#define SECVAULT_MINOR (25)

struct secvault_dev {
    unsigned long size;
    char *key;
    struct cdev cdev;
    struct semaphore sem; 
};

int secvault_major = SECVAULT_MAJOR; 
int secvault_minor = SECVAULT_MINOR;

//dev_t dev;
struct cdev cdev; 

// struct task_struct *current;

/* struct file_operations secvault_dev_fops = {
    .owner   = THIS_MODULE, 
    .read    = secvault_read,
    .write   = secvault_write,
    .ioctl   = secvault_ioctl,
    .open    = secvault_open,
    .release = secvault_release,
}; */


// unsigned int iminor(struct inode *inode); 
// unsigned int imajor(struct inode *inode);

long secvault_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    // int err = 0;
    int retval = 0;
    
    /*
    if (_IOC_TYPE(cmd) != SECVAULT_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > SECVAULT_IOC_MAXNR) return -ENOTTY;
    */

    switch (cmd) {
        case SECVAULT_IOC_CREATE:
            printk(KERN_INFO "secvault: create command\n");
            break;
        case SECVAULT_IOC_CHANGEKEY:
            printk(KERN_INFO "secvault: changekey command\n");
            break;
        case SECVAULT_IOC_DELETE:
            printk(KERN_INFO "secvault: delete command\n");
            break;
        case SECVAULT_IOC_SIZE:
            printk(KERN_INFO "secvault: size command\n");
            break;
        case SECVAULT_IOC_REMOVE:
            printk(KERN_INFO "secvault: remove command\n");
            break;
        default: 
            printk(KERN_INFO "secvault: wrong command\n");
            retval = 1;
            break;
    }

    return retval;
}

struct file_operations secvault_control_fops = {
    .owner   = THIS_MODULE, 
    .unlocked_ioctl = secvault_ioctl,
};


/* ssize_t secvault_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct secvault_dev *dev = filp->private_data;
    struct secvault_qset *dptr;
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM; 

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; q_pos = rest % quantum;

    dptr = secvault_follow(dev, item);
    if (dptr == NULL) 
        goto out;
    if (!dptr->data) {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    } *f_pos += count;
    retval = count;

    if (dev->size < *f_pos)
        dev->size = *f_pos;

out:
    up(&dev->sem);
    return retval;
} 

 ssize_t secvault_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct secvault_dev *dev = filp->private_data;
    struct secvault_qset *dptr; 
    int quantum = dev->quantum, qset = dev->qset; 
    int itemsize = quantum * qset; 
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; q_pos = rest % quantum;

    dptr = scull_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_ps])
        goto out;

    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
} 

static void secvault_setup_cdev(struct secvault_dev *dev, int index) {
    int err, devno = MKDEV(secvault_major, securevalt_minor + index);

    cdev_init(&dev->cdev, &secvault_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &secvault_fops;
    err = cdev_add (&dev->cdev, devno, 1);

    if (err) {
        printk(KERN_NOTICE "Error %d adding secvault %d", err. index);
    }
}*/

// unsigned long copy_to_user(void __user *to, const void *from, unsigned long count);
// unsigned long copy_from_user(void __user *to, const void __user *from, unsigned long count);

static void __exit secvault_cleanup_module(void) {
    dev_t devno = MKDEV(secvault_major, secvault_minor); 

    unregister_chrdev_region(devno, 1); 
    cdev_del(&cdev);

    printk ("Bye World! secvault unloading...\n");
}


static int __init secvault_init_module(void) {
    int result;//, i; 
    dev_t dev = 0;

    printk("Hello World! secvault loading!\n");
    dev = MKDEV(secvault_major, secvault_minor);
    result = register_chrdev_region(dev, 1, "sv_ctl\n");

    if (result < 0) {
        printk(KERN_WARNING "secvault: can't get major %d\n", secvault_major);
        return result;
    }

    // int err, devno = MKDEV(secvault_major, securevalt_minor);

    cdev_init(&cdev, &secvault_control_fops);
    cdev.owner = THIS_MODULE;
    cdev.ops = &secvault_control_fops;
    result = cdev_add(&cdev, dev, 1);

    if (result) {
        printk(KERN_NOTICE "Error %d adding sv_ctl\n", result);
        goto fail;
    }
/*

    secvault_devices = kmalloc(secvault_nr_devs * sizeof(struct secvault_dev), GFP_KERNEL);
    if (!secvault_devices) {
        result = -ENOMEM;
        goto fail;
    }   

    memset(secvault_devices, 0, secvault_nr_devs, * sizeof(struct secvault_dev));

    for (i = o; i < secvault_nr_devs; i++) {
        init_MUTEX(&secvault_devices[i].sem);
        secvault_setup_cdev(&secvault_devices[i], i);
        // secvault_devices[i] 
    }

    dev = MKDEV(secvault_major, secvault_minor + secvault_nr_devs);
    dev += secvault_p_init(dev);
    dev += secvault_access_init(dev);
*/
	
    return 0;

fail: 
    secvault_cleanup_module();
    return result;
}

module_init(secvault_init_module);
module_exit(secvault_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raphael Gruber");
