#include <linux/slab.h>
//#include <linux/string.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "secvault.h"

struct secvault_dev {
    void *data;            // pointer to the allocated memory
    unsigned long size;     // size of the allocated memory
    char *key;              // xor encryption key
    struct cdev cdev;       
    struct mutex mutex; 
};

int secvault_major = SECVAULT_MAJOR; 
int secvault_minor = SECVAULT_MINOR;
int secvault_nr_devs = SECVAULT_NR_DEVS;

struct cdev svctl_cdev; 

struct secvault_dev *secvault_devices;

static long delete(long id) {
    memset(secvault_devices[id].data, 0, secvault_devices[id].size); 
    return 0;
}

static long remove(long id) {
    struct secvault_dev dev = secvault_devices[id];

    if (dev.data) {
        kfree(dev.data);
    }

    dev.data = NULL;
    dev.key = NULL;
    dev.size = 0;

    return 0;
}

static long size(long id) {
    return secvault_devices[id].size;
}

static long changekey(struct dev_params params) {
    secvault_devices[params.id].key = params.key;
    return 0;
}

static long create(struct dev_params params) {
    long result = 0;

    struct secvault_dev *dev;
    dev = &secvault_devices[params.id];

    dev->key  = params.key; 
    dev->size = params.size; 
    dev->data = kmalloc(params.size, GFP_KERNEL);

    if (!dev->data) {
        result = -ENOMEM;
    } else {
        memset(dev->data, 0, dev->size); 
    }

    return result;
}

long secvault_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    long retval = 0;
    int id;
    struct dev_params params;
    
    if (_IOC_TYPE(cmd) != SECVAULT_IOC_MAGIC) { 
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > SECVAULT_IOC_MAXNR) { 
        return -ENOTTY;
    }

    switch (cmd) {
        case SECVAULT_IOC_CREATE:
            retval = copy_from_user(&params, (void __user *)arg, sizeof(params));
            if (retval == 0) {
                retval = create(params);
                printk(KERN_INFO "secvault: create device %d with size %d\n", (int)params.id, (int)params.size);
            }
            break;
        case SECVAULT_IOC_CHANGEKEY:
            retval = copy_from_user(&params, (void __user *)arg, sizeof(params));
            printk(KERN_INFO "secvault: changekey %d\n", params.id);
            if (retval == 0) {
                retval = changekey(params);
            }
            break;
        case SECVAULT_IOC_DELETE:
                retval = __get_user(id, (int __user *)arg);
            printk(KERN_INFO "secvault: delete %d\n", id);
            if (retval == 0) {
                retval = delete(id);
            }
            break;
        case SECVAULT_IOC_SIZE:
            retval = __get_user(id, (int __user *)arg);
            printk(KERN_INFO "secvault: size %d\n", id);
            if (retval == 0) {
                int tmp = size(id);
                retval = __put_user(tmp, (int __user *)arg);
            }
            break;
        case SECVAULT_IOC_REMOVE:
            retval = __get_user(id, (int __user *)arg);
            printk(KERN_INFO "secvault: remove %d\n", id);
            if (retval == 0) {
                retval = remove(id);
            }
            break;
        default: 
            printk(KERN_INFO "secvault: wrong command\n");
            retval = 1;
            break;
    }

    return retval;
}

ssize_t secvault_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {

    struct secvault_dev *dev;
    dev = filp->private_data;

    printk(KERN_INFO "secvault: write size: %d, f_pos: %d, count: %d\n", (int)dev->size, (int)*f_pos, (int)count);

    ssize_t retval = -ENOMEM; 

    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }
    if (*f_pos + count > dev->size) {
        retval = -ENOSPC;
        goto out;
        //count = dev->size - *f_pos;
    }
    if (copy_from_user(dev->data + (long)*f_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    } 

    *f_pos += count;
    retval = count;

out:
    mutex_unlock(&dev->mutex);
    return retval;
} 

ssize_t secvault_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {

    printk(KERN_INFO "secvault: reading, f_pos: %d, count: %d\n", (int)*f_pos, (int)count);

    struct secvault_dev *dev;
    dev = filp->private_data;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }
    if (*f_pos >= dev->size) {
        goto out;
    }
    if (*f_pos + count > dev->size) {
        count = dev->size - *f_pos;
    }
    if (copy_to_user(buf, dev->data + (long)*f_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    mutex_unlock(&dev->mutex);
    return retval;
} 

loff_t secvault_seek(struct file *filp, loff_t off, int whence) {
    printk(KERN_INFO "secvault: seek, f_pos: %d, offset: %d\n", (int)filp->f_pos, (int)off);

    struct secvault_dev *dev = filp->private_data; 
    loff_t newpos;


    switch (whence) {
        case 0: // SEEK_SET
            newpos = off;
            break;
        case 1: // SEEK_CUR
            newpos = filp->f_pos + off;
            break;
        case 2: // SEEK_END 
            newpos = dev->size + off;
            break;
        default:
            return -EINVAL;
    }

    if (newpos < 0) {
        return -EINVAL;
    }

    filp->f_pos = newpos;
    return newpos;
}

int secvault_open(struct inode *inode, struct file *filp) {
    struct secvault_dev *dev;

    dev = container_of(inode->i_cdev, struct secvault_dev, cdev);
    printk(KERN_INFO "secvault: open, size: %d\n", (int)dev->size);
    filp->private_data = dev;

    return 0;
}

int secvault_release(struct inode *inode, struct file *filp) {
    return 0;
}

struct file_operations secvault_control_fops = {
    .owner   = THIS_MODULE, 
    .unlocked_ioctl = secvault_ioctl,
};

struct file_operations secvault_dev_fops = {
    .owner   = THIS_MODULE, 
    .open    = secvault_open,
    .release = secvault_release,
    .llseek  = secvault_seek,
    .read    = secvault_read,
    .write   = secvault_write,
}; 

void secvault_cleanup_module(void) {
    int i;
    dev_t dev = MKDEV(secvault_major, secvault_minor); 

    if (secvault_devices) {
        for (i = 0; i < secvault_nr_devs; i++) {
            if (secvault_devices[i].data) {
                kfree(secvault_devices[i].data);
            }
            cdev_del(&secvault_devices[i].cdev);
        }
        kfree(secvault_devices);
    }

    cdev_del(&svctl_cdev);

    unregister_chrdev_region(dev, 1 + secvault_nr_devs); 

    printk("secvault: cleanup finished\n");
}

static void secvault_setup_cdev(struct secvault_dev *dev, int index) {
    int err, devno = MKDEV(secvault_major, secvault_minor + index + 1);

    cdev_init(&dev->cdev, &secvault_dev_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &secvault_dev_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding secvault %d", err, index);
    }
}

static int __init secvault_init_module(void) {
    int result, i;
    dev_t dev = 0;

    printk("secvault: initializing\n");
    dev = MKDEV(secvault_major, secvault_minor);
    result = register_chrdev_region(dev, 1 + secvault_nr_devs, "secvault");

    if (result < 0) {
        printk(KERN_WARNING "secvault: can't get major %d\n", secvault_major);
        return result;
    }

    cdev_init(&svctl_cdev, &secvault_control_fops);
    svctl_cdev.owner = THIS_MODULE;
    svctl_cdev.ops = &secvault_control_fops;
    result = cdev_add(&svctl_cdev, dev, 1);

    if (result) {
        printk(KERN_NOTICE "Error %d adding sv_ctl\n", result);
        goto fail;
    }

    secvault_devices = kmalloc(secvault_nr_devs * sizeof(struct secvault_dev), GFP_KERNEL);
    if (!secvault_devices) {
        result = -ENOMEM;
        goto fail;
    }   

    memset(secvault_devices, 0, secvault_nr_devs * sizeof(struct secvault_dev));

    for (i = 0; i < secvault_nr_devs; i++) {
        mutex_init(&secvault_devices[i].mutex);
        secvault_setup_cdev(&secvault_devices[i], i);
    }

    return 0;

fail: 
    secvault_cleanup_module();
    return result;
}

module_init(secvault_init_module);
module_exit(secvault_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raphael Gruber");
