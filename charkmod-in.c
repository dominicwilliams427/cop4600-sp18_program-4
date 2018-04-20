/**
 * File:	charkmod-in.c
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
#define DEVICE_NAME "charkmod-in"	// Device name.
#define MAX_SIZE    1024		// Max buffer size.


/**
 * Mod Info:	The module uses a GPL to avoid tainting the kernel.
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arati Banerjee, Huong Dang, and Jorge B. Nunez");


/**
 * Important variables that store data and keep track of relevant information.
 */
static char *message = "Undefeated 2018 National Champions UCF";
static int  major_number;
static char data[MAX_SIZE];
static int  data_size;
static DEFINE_MUTEX(buffer_mutex);
EXPORT_SYMBOL(buffer_mutex);
EXPORT_SYMBOL(data);
EXPORT_SYMBOL(data_size);


/**
 * Prototype functions for file operations.
 */
static int     open(struct inode *, struct file *);
static int     close(struct inode *, struct file *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);


/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
{
	.owner   = THIS_MODULE,
	.open    = open,
	.release = close,
	.write   = write,
};


/**
 * Initializes module at installation
 */
int init_module(void)
{
	int i;

	printk(KERN_INFO "charkmod-in: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "charkmod-in could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod-in: registered correctly with major number %d\n", major_number);

	// Initialize all data bytes to '\0'.
	data_size = 0;
	for (i = 0; i < MAX_SIZE; i++) {
		data[i] = '\0';
	}

	mutex_init(&buffer_mutex);

	return 0;
}


/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "charkmod-in: removing module.\n");

	unregister_chrdev(major_number, DEVICE_NAME);

	mutex_destroy(&buffer_mutex);

	return;
}


/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&buffer_mutex)) {
		printk(KERN_ALERT "charkmod-in: device in use by another process\n");
		return -EBUSY;
	}

	printk(KERN_INFO "charkmod-in: device opened for writing.\n");

	return 0;
}


/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&buffer_mutex);

	printk(KERN_INFO "charkmod-in: device closed.\n");

	return 0;
}


/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int i, j, k, exists, prev_size, upper_limit;
	char checker[3], temp[MAX_SIZE];

	printk(KERN_INFO "charkmod-in: something wrote to device.\n");

	// Clear temp buffer.
	for (i = 0; i < MAX_SIZE; i++) {
		temp[i] = '\0';
	}

	// Count how many times 'UCF' shows up in input string.
	for (i = 0, exists = 0; i < len - 2; i++) {
		checker[0] = buffer[i];
		checker[1] = buffer[i+1];
		checker[2] = buffer[i+2];
		if (checker[0] == 'U' && checker[1] == 'C' && checker[2] == 'F') {
			exists++;
		}
		checker[0] = '\0';
		checker[1] = '\0';
		checker[2] = '\0';
	}

	printk(KERN_INFO "charkmod-in: there exist %d instances of \'UCF\' in the input.\n", exists);

	// Writes the data to the device
	for (i = data_size, prev_size = data_size, j = 0, upper_limit = len; i < MAX_SIZE; i++) {
		if (i >= prev_size + upper_limit)
			data[i] = '\0';
		else if (j < len-2 && buffer[j] == 'U' && buffer[j+1] == 'C' && buffer[j+2] == 'F') {
			for (k = 0; message[k] != '\0' && i < MAX_SIZE; i++, k++, data_size++, upper_limit++)
				data[i] = message[k];
			i--;
			j = j + 3;
			upper_limit = upper_limit - 3;
		}
		else {
			data[i] = buffer[j];
			j++;
			data_size++;
		}
	}
	
	// Sends message to kernel when there is less space than offered data
	if (data_size == MAX_SIZE) {
		printk(KERN_INFO "charkmod-in: not enough space! Dropping what's left.\n");
	}
	
	// Returns the count of the number of bytes attempted to be written
	return len;
}
