#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "rw_dev.h"

static RW_DEV_EXT rw_dev_ext;

//
// Initialize File Operations
//

static struct file_operations fops = 
{
	.open 		= rw_dev_open,
	.read 		= rw_dev_read,
	.write 		= rw_dev_write,
	.release 	= rw_dev_release,
};

//
// rw_dev driver exit routine
//

static void rw_dev_exit(void)
{
	printk(KERN_INFO "rw_dev_exit - entry\n");

	if (!IS_ERR(rw_dev_ext.rw_dev_device))
	{
		device_destroy(rw_dev_ext.rw_dev_class, MKDEV(rw_dev_ext.major_number, 0));
	}

	if (!IS_ERR(rw_dev_ext.rw_dev_class))
	{
		class_destroy(rw_dev_ext.rw_dev_class);
	}

	if (rw_dev_ext.major_number >= 0)
	{
		unregister_chrdev(rw_dev_ext.major_number, RW_DEV_NAME);
	}

	printk(KERN_INFO "rw_dev_exit - exit\n");
}

//
// rw_dev driver entry routine
//

static int rw_dev_init(void)
{
	int status = 0;

	printk(KERN_INFO "rw_dev_init - entry\n");

	//
	// Create read mutex lock
	//

	sema_init(&rw_dev_ext.read_sema, 1);

	//
	// Create write mutex lock
	//

	sema_init(&rw_dev_ext.write_sema, 1);

	//
	// Set num_readers to 0
	//

	rw_dev_ext.num_readers = 0;

	//
	// Register Character Device
	//

	rw_dev_ext.major_number = register_chrdev(0, RW_DEV_NAME, &fops);

	if (rw_dev_ext.major_number < 0)
	{
		printk(KERN_ERR "Failed to register rw_dev\n");

		status = rw_dev_ext.major_number;

		goto Exit;
	}

	//
	// Create RW_DEV class 
	//

	rw_dev_ext.rw_dev_class = class_create(THIS_MODULE, RW_DEV_CLASS_NAME);

	if (IS_ERR(rw_dev_ext.rw_dev_class))
	{
		printk(KERN_ERR "Failed to create class\n");

		status = PTR_ERR(rw_dev_ext.rw_dev_class);

		goto Exit;
	}

	//
	// Create RW_DEV Device
	//

	rw_dev_ext.rw_dev_device = device_create(rw_dev_ext.rw_dev_class, NULL, MKDEV(rw_dev_ext.major_number, 0), NULL, RW_DEV_NAME);

	if (IS_ERR(rw_dev_ext.rw_dev_device))
	{
		printk(KERN_ERR "Failed to create device\n");

		status = PTR_ERR(rw_dev_ext.rw_dev_device);

		goto Exit;
	}


Exit:

	if (status != 0)
	{
		rw_dev_exit();
	}

	printk(KERN_INFO "rw_dev_init - exit\n");

	return status;	
}

//
// rw_dev open routine
//

static int rw_dev_open(struct inode *pInode, struct file *pFile)
{
	printk(KERN_INFO "rw_dev_open - entry\n");

	printk(KERN_INFO "rw_dev_open - exit\n");

	return 0;
}

//
// rw_dev close routine
//

static int rw_dev_release(struct inode *pInode, struct file *pFile)
{
	printk(KERN_INFO "rw_dev_release - entry\n");

	printk(KERN_INFO "rw_dev_release - exit\n");

	return 0;
}

//
// rw_dev read routine
//

static ssize_t rw_dev_read(struct file *pFile, char *pBuffer, size_t len, loff_t *pLoff)
{
	ssize_t size = 0;

	printk(KERN_INFO "rw_dev_read - entry\n");

	//
	// Check if the arguments passed are valid
	//

	if (pBuffer == NULL || 
	    len == 0 || 
	    pLoff == NULL || 
	    *pLoff >= BUFFER_SIZE ||
	    (*pLoff + len) >= BUFFER_SIZE)
	{
		printk(KERN_ERR "Invalid arguments");

		goto Exit;
	}

	//
	// Acquire read semaphore and increment number of readers.
	//

	down(&rw_dev_ext.read_sema);

	rw_dev_ext.num_readers++;

	//
	// If number of readers == 1 then acquire write lock
	//

	if (rw_dev_ext.num_readers == 1)
	{
		down(&rw_dev_ext.write_sema);
	}

	//
	// Now release reader lock
	//

	up(&rw_dev_ext.read_sema);

	//
	// Now copy the message to the buffer
	//

	size = copy_to_user(pBuffer, rw_dev_ext.buffer + *pLoff, len); 

	if (size != 0)
	{
		printk(KERN_ERR "Failed to copy to user space");

		size = len - size;
	}
	else
	{
		size = len;
	}

	//
	// Acquire read semaphore and decrement number of readers
	//

	down(&rw_dev_ext.read_sema);

	rw_dev_ext.num_readers--;

	//
	// If number of readers == 0 then release write lock
	//

	if (rw_dev_ext.num_readers == 0)
	{
		up(&rw_dev_ext.write_sema);
	}

	//
	// Now release reader lock.
	//

	up(&rw_dev_ext.read_sema);

Exit:

	printk(KERN_INFO "rw_dev_read - exit\n");

	return size;
}

//
// rw_dev write routine
//

static ssize_t rw_dev_write(struct file *pFile, const char *pBuffer, size_t len, loff_t *pLoff)
{
	ssize_t size = 0;

	printk(KERN_INFO "rw_dev_write - entry\n");

	//
	// Check if the arguments passed are valid
	//

	if (pBuffer == NULL || 
	    len == 0 || 
	    pLoff == NULL || 
	    *pLoff >= BUFFER_SIZE ||
	    (*pLoff + len) >= BUFFER_SIZE)
	{
		printk(KERN_ERR "Invalid arguments");

		goto Exit;
	}

	//
	// Acquire write lock
	//

	down(&rw_dev_ext.write_sema);

	size = copy_from_user(rw_dev_ext.buffer + *pLoff, pBuffer, len); 

	if (size != 0)
	{
		printk(KERN_ERR "Failed to copy from user space");

		size = len - size;
	}
	else
	{
		size = len;
	}

	//
	// Release write lock
	//

	up(&rw_dev_ext.write_sema);

Exit:

	printk(KERN_INFO "rw_dev_write - exit\n");

	return size;
}

//
// Macros for registering driver entry and exit routines
//

module_init(rw_dev_init);

module_exit(rw_dev_exit);

//
// Module license and description
//

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Shanthanand R Kutuva");

MODULE_DESCRIPTION("Character device with reader writer lock");

MODULE_VERSION("1.0");
