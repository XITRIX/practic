#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kthread.h>

#define SAW_LIMIT 15

static void *acme_buf;
static int acme_bufsize = 256;
static int write_caret = 0;
static int read_caret = 0;

DEFINE_RWLOCK( lock );

static int acme_count = 1;
static dev_t acme_dev = MKDEV(202, 128);

static struct cdev acme_cdev; 

static struct task_struct *ts;
static int counter = 0;

int thread(void *data){
	for (;;){
		char buf[4];
		sprintf(buf,"%d",counter);
		memcpy(acme_buf + write_caret /*to*/ , buf /*from*/ , strlen(buf));
		write_caret+=strlen(buf);
		write_caret %= acme_bufsize;
		counter++;
		counter %= SAW_LIMIT;

		if (kthread_should_stop()) break;
	}
	return 0;
}

static ssize_t 
acme_open(struct inode *_inode, struct file *_file){
	//printk(KERN_INFO"OPEN!!!\n");
	return 0;
}

static ssize_t
acme_read(struct file *file, char __user * buf, size_t count, loff_t * ppos)
{
	//printk("READ!!!\n");
	int i = 0;
	while (read_caret != write_caret){
		if (read_caret > write_caret+1) 
			write_unlock( &lock );
		copy_to_user (buf+i /* to */ , acme_buf + *ppos + read_caret /* from */ , 1);
		read_caret++;
		read_caret %= acme_bufsize;
		i++;
	}

	write_unlock( &lock );
	return i;
}

static ssize_t
acme_write(struct file *file, const char __user *buf, size_t count,
	   loff_t *ppos)
{
	//printk("WRITE!!!\n");
	/*int i = 0;
	while (i < count){
		if (write_caret >= read_caret-1) 
			write_lock( &lock );
		copy_from_user(acme_buf + *ppos + write_caret /*to*-/ , buf + i /*from*-/ , 1);
		write_caret++;
		write_caret %= acme_bufsize;
		i++;
	}*/

	//return count; 
		return 0;
}

static const struct file_operations acme_fops = {
	.owner = THIS_MODULE,
	.read = acme_read,
	.write = acme_write,
	.open = acme_open,
};

static int __init acme_init(void)
{
	int err;
	acme_buf = kmalloc(acme_bufsize, GFP_ATOMIC);
	memset(acme_buf,0,acme_bufsize);

	if (!acme_buf) {
		err = -ENOMEM;
		goto err_exit;
	}

	if (register_chrdev_region(acme_dev, acme_count, "acme")) {
		err = -ENODEV;
	}

	cdev_init(&acme_cdev, &acme_fops);

	if (cdev_add(&acme_cdev, acme_dev, acme_count)) {
		err = -ENODEV;
		goto err_dev_unregister;
	}

	ts = kthread_run(thread,NULL,"write thread");

	return 0;

 err_dev_unregister:
	unregister_chrdev_region(acme_dev, acme_count);
 err_exit:
	return err;
}

static void __exit acme_exit(void)
{
	kthread_stop(ts);
	kfree(acme_buf);
	cdev_del(&acme_cdev);
	unregister_chrdev_region(acme_dev, acme_count);
}

module_init(acme_init);
module_exit(acme_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example character driver");
MODULE_AUTHOR("Free Electrons");