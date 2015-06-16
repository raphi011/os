#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>

struct task_struct *current;

static int __init tm_init(void)
{
	printk ("Hello World! I am a simple tm (test module)!\n");
	return 0;
}

static void __exit tm_exit(void)
{
	printk ("Bye World! tm unloading...\n");
}


module_init(tm_init);
module_exit(tm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raphael Gruber")
