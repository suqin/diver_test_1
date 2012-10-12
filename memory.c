#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk()*/
#include <linux/slab.h>   /* kmalloc()*/
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>  /*size_t*/
#include <linux/proc_fs.h>
#include <linux/fcntl.h>  /*O_ACCMODE*/
#include <asm/system.h>   /* cli(),*_flag*/
#include <asm/uaccess.h>  /*copy_from...()*/
MODULE_LICENSE("Dual BSD/GPL");


int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct indoe *inode, struct file *filp) {return 0;}
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp,char *buf,size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);
struct file_operations memory_fops = {
	read: memory_read,
	write: memory_write,
	open: memory_open,
	release: memory_release
};
module_init(memory_init);
module_exit(memory_exit);
int memory_major = 60;
char *memory_buffer;



int memory_init(void) {
	int resault;
	resault = register_chrdev(memory_major, "memory", &memory_fops);
	if (resault < 0) {
		printk("cannot obtain major num: %d\n",
			memory_major);
		return resault;
	}
	memory_buffer = kmalloc(1, GFP_KERNEL);
	if (!memory_buffer) {
		resault = -ENOMEM;
		goto fail;
	}
	memset(memory_buffer, 0, 1);
	printk("<1>memory malloc succeed\n");
	return 0;
	fail:
		memory_exit();
		return resault;
}
	


int memory_open(struct inode *inode, struct file *file) {
	return 0;
}



void memory_exit(void) {
	unregister_chrdev(memory_major, "memory");
	if(memory_buffer) {
		kfree(memory_buffer);
	}
	printk("<1>Removing memory device\n");
}


ssize_t memory_read(struct file *filp, char *buf,
		    size_t count, loff_t *f_pos) {
	copy_to_user(buf,memory_buffer,1);
	if (*f_pos == 0) {
		*f_pos+=1;
		return 1;
	} else {
		return 0;
	}
}


ssize_t memory_write(struct file *filp, char *buf,
		     size_t count, loff_t *fpos) {
	char *temp;
	temp = buf+count-1;
	copy_from_user(memory_buffer,temp,1);
	return 1;
}
