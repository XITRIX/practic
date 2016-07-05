#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

static unsigned int counter = 0;

static void **sys_call_table = NULL;

static asmlinkage long (*old_open) (const char __user *filename, int flags, umode_t mode);

static asmlinkage long rick_open(const char __user *filename, int flags, umode_t mode)
{
        counter++;
        printk(KERN_INFO"Counted %d\n",counter);
        return old_open(filename, flags, mode);
}

static int __init set_rick(void)
{
        sys_call_table = (void **)kallsyms_lookup_name("sys_call_table");
        old_open = sys_call_table[__NR_open];
        sys_call_table[__NR_open] = rick_open;
        printk(KERN_INFO"Run open() counter\n", old_open, rick_open);
        return 0;
}

static void __exit unset_rick(void)
{
        printk(KERN_INFO"Stop open() counter\n");
        sys_call_table[__NR_open] = old_open;
}

module_init(set_rick);
module_exit(unset_rick);