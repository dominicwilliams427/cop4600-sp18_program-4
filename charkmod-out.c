/**
 * File:	charkmod-out.c
 * Author:	Arati Banerjee, Huong Dang, Jorge Brandon Nunez
 * Class:	COP4600-SP18
 * Professor:	Dr. Gerber
 * Due Date:	2018-04-20
 */


#include <linux/module.h>		// Core header for modules.
#include <linux/device.h>		// Supports driver model.
#include <linux/kernel.h>		// Kernel header for convenience.
#include <linux/fs.h>			// File-system support.
#include <linux/uaccess.h>		// User access copy function support.
#include <linux/mutex.h>		// Mutex library for synchronization.
#define DEVICE_NAME "charkmod-out"	// Device name.
#define MAX_SIZE    1024		// Max buffer size. 


/**
 * Mod Info:	The module uses a GPL to avoid tainting the kernel.
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arati Banerjee, Huong Dang, and Jorge B. Nunez");


/**
 * Important variables that store data and keep track of relevant information.
 */
static int  major_number;
static char temp[MAX_SIZE];
extern char data[MAX_SIZE];
extern int  data_size;
extern struct mutex buffer_mutex;


/**
 * Prototype functions for file operations.
 */
static int     open(struct inode *, struct file *);
static int     close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);


/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
{
	.owner   = THIS_MODULE,
	.open    = open,
	.release = close,
	.read    = read,
};


int init_module(void)
{
	int i;
	
	printk(KERN_INFO "charkmod-out: installing module.\n");
	
	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "charkmod-out could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod-out: registered correctly with major number %d\n", major_number);
	
	// Initialize all temp bytes to '\0'.
	for (i = 0; i < MAX_SIZE; i++) {
		temp[i] = '\0';
	}
	
	return 0;
}


void cleanup_module(void)
{
	printk(KERN_INFO "charkmod-out: removing module.\n");
	
	unregister_chrdev(major_number, DEVICE_NAME);
	
	return;
}


static int open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&buffer_mutex)) {
		printk(KERN_ALERT "charkmod-out: device busy with another process");
		return -EBUSY;
	}
	
	printk(KERN_INFO "charkmod-out: device opened for reading.\n");
	
	return 0;
}


static int close(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&buffer_mutex);
	
	printk(KERN_INFO "charkmod-out: device closed.\n");
	
	return 0;
}


static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int i, j;
	int limit = MAX_SIZE, error = 0;
	
	printk(KERN_INFO "charkmod-out: something read from device.\n");
	
	if (data_size == 0) {
		printk(KERN_INFO "charkmod-out: tried to read empty buffer!\n");
	}
	
	if (len < data_size) {
		limit = len;
		data_size = data_size - len;
	} else {
		limit = data_size;
		data_size = 0;
	}
	
	error = copy_to_user(buffer, data, limit);
	if (error != 0) {
		printk(KERN_INFO "charkmod-out: error copying to user!\n");
		return -EFAULT;
	}
	
	
	for (i = limit, j = 0; i < MAX_SIZE; i++, j++) {
		temp[j] = data[i];
	}
	
	for ( ; j < MAX_SIZE; j++) {
		temp[j] = '\0';
	}
	
	for (i = 0; i < MAX_SIZE; i++) {
		data[i] = temp[i];
	}
	
	return limit;
}
