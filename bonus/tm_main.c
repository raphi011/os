#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>


#define SECUREVAULT_MAJOR (231) // use 231 as major device number!
#define SECUREVAULT_MINOR (25)

struct securevault_dev {


    unsignd long size;
    char *key;
    struct cdev cdev;

}

int securevault_major = SECUREVAULT_MAJOR; 
int securevault_minor = SECUREVAULT_MINOR;

dev_t dev;

// struct task_struct *current;
struct file_operations securevault_fops = {
    .owner   = THIS_MODULE, 
    .read    = securevault_read,
    .write   = securevault_write,
    .ioctl   = securevault_ioctl,
    .open    = securevault_open,
    .release = securevault_release,
};

// unsigned int iminor(struct inode *inode); 
// unsigned int imajor(struct inode *inode);

ssize_t securevault_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct securevault_dev *dev = filp->private_data;
    struct securevault_qset *dptr;
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM; 

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; q_pos = rest % quantum;

    dptr = securevault_follow(dev, item);
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

ssize_t securevault_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct securevault_dev *dev = filp->private_data;
    struct securevault_qset *dptr; 
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

static void securevault_setup_cdev(struct securevault_dev *dev, int index) {
    int err, devno = MKDEV(securevault_major, securevalt_minor + index);

    cdev_init(&dev->cdev, &securevault_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &securevault_fops;
    err = cdev_add (&dev->cdev, devno, 1);

    if (err) {
        printk(KERN_NOTICE "Error %d adding securevault %d", err. index);
    }
}

// unsigned long copy_to_user(void __user *to, const void *from, unsigned long count);
// unsigned long copy_from_user(void __user *to, const void __user *from, unsigned long count);

static int __init tm_init(void)
{
    printk("Hello World! I am a simple tm (test module)!\n");
    dev = MKDEV(securevault_major, securevalt_minor);
    result = register_chrdev_region(dev, 1, "sv_ctl");

    if (result < 0) {
        printk(KERN_WARNING "securevault: can't get major %d\n", securevault_major);
        return result;
    }
    
    struct cdev *my_cdev = cdev_alloc();
    my_cdev->ops = &my_fops;
    my_cdev->owner = THIS_MODULE;

    // only call this when we are ready to handle all operations 
    // cdev_add(struct cdev *dev, dev_t num, unsigned int count);

	
    return 0;
}

static void __exit tm_exit(void)
{
    // void cdev_del(struct cdev *dev);
    ma
    // void unregister_chrdev_region(dev_t first, unsigned int count);
	printk ("Bye World! tm unloading...\n");
}


module_init(tm_init);
module_exit(tm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raphael Gruber")
