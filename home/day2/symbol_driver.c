#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

/* Physical address of the imaginary acme device */

static void *acme_buf;
static int acme_bufsize = 255;
static int write_caret = 0;
static int read_caret = 0;

DEFINE_RWLOCK( lock );

static int acme_count = 1;
static dev_t acme_dev = MKDEV(202, 128);

static struct cdev acme_cdev;

static ssize_t
acme_read(struct file *file, char __user * buf, size_t count, loff_t * ppos)
{
	/* The acme_buf address corresponds to a device I/O memory area */
	/* of size acme_bufsize, obtained with ioremap() */
	printk("READ!!!");
	/*int remaining_size, transfer_size;

	//write_unlock( &lock );

	remaining_size = acme_bufsize - (int)(*ppos) - read_caret;
				/* bytes left to transfer *-/
	if (remaining_size == 0) {
				/* All read, returning 0 (End Of File) *-/
		return 0;
	}

	/* Size of this transfer *-/
	transfer_size = min_t(int, remaining_size, count);

	if (copy_to_user
	    (buf /* to *-/ , acme_buf + *ppos + read_caret /* from *-/ , transfer_size)) {
		return -EFAULT;
	} else {		/* Increase the position in the open file *-/
		read_caret += transfer_size;
		read_caret %= acme_bufsize;
		*ppos += transfer_size;
		return transfer_size;
	}*/
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
	printk("WRITE!!!");
	/*int remaining_bytes;
	//write_lock( &lock );

	/* Number of bytes not written yet in the device *-/
	remaining_bytes = acme_bufsize - (*ppos);

	if (count > remaining_bytes) {
		/* Can't write beyond the end of the device *-/
		return -EIO;
	}

	if (copy_from_user(acme_buf + *ppos + write_caret /*to*-/ , buf /*from*-/ , count)) {
		return -EFAULT;
	} else {
		/* Increase the position in the open file *-/
		write_caret+=count;
		write_caret %= acme_bufsize;
		*ppos += count;
		return count;
	}*/
	int i = 0;
	while (i < count){
		if (write_caret >= read_caret-1) 
			write_lock( &lock );
		copy_from_user(acme_buf + *ppos + write_caret /*to*/ , buf + i /*from*/ , 1);
		write_caret++;
		write_caret %= acme_bufsize;
		i++;
	}

	return count;
}

static const struct file_operations acme_fops = {
	.owner = THIS_MODULE,
	.read = acme_read,
	.write = acme_write,
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

	return 0;

 err_dev_unregister:
	unregister_chrdev_region(acme_dev, acme_count);
 err_exit:
	return err;
}

static void __exit acme_exit(void)
{
	kfree(acme_buf);
	cdev_del(&acme_cdev);
	unregister_chrdev_region(acme_dev, acme_count);
}

module_init(acme_init);
module_exit(acme_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example character driver");
MODULE_AUTHOR("Free Electrons");