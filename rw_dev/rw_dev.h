#ifndef _RW_DEV_H_
#define _RW_DEV_H_

#include <linux/semaphore.h>

#define RW_DEV_NAME 		"rw_dev"
#define RW_DEV_CLASS_NAME  	"rw_dev_char"
#define BUFFER_SIZE		256

typedef struct _RW_DEV_EXT
{
	int major_number;

	struct class* rw_dev_class;

	struct device* rw_dev_device;

	struct semaphore read_sema;

	struct semaphore write_sema;

	unsigned int num_readers;

	char buffer[BUFFER_SIZE];

} RW_DEV_EXT, *PRW_DEV_EXT;

static int rw_dev_open(struct inode *pInode, struct file *pFile);

static int rw_dev_release(struct inode *pInode, struct file *pFile);

static ssize_t rw_dev_read(struct file *pFile, char *pBuffer, size_t Size, loff_t *pLoff);

static ssize_t rw_dev_write(struct file *pFile, const char *pBuffer, size_t Size, loff_t *pLoff); 

#endif
