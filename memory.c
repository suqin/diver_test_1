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
#include <linux/semaphore.h> /*linux 2.6.32*/
//#include <asm/semaphore.h>   /*linux 2.6.32以下版本*/
#include <linux/fcntl.h>  /*O_ACCMODE*/>
#include "ioct.h"
MODULE_LICENSE("Dual BSD/GPL");

int memory_open(struct inode *inode, struct file *filp);

int memory_release(struct indoe *inode, struct file *filp) {return 0;}

ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);

ssize_t memory_write(struct file *filp,char *buf,size_t count, loff_t *f_pos);

int memory_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

void memory_exit(void);

int memory_init(void);

struct file_operations memory_fops = {
	read: memory_read,
	write: memory_write,
	open: memory_open,
	release: memory_release,
	ioctl: memory_ioctl
};

module_init(memory_init);

module_exit(memory_exit);

int memory_major = 60;

struct mem_dev {
	void *data;              //数据区指针
	int size;               //数据最大长度
	int length;            //当前数据长度
	struct semaphore sem; //互斥旗标
};
struct mem_dev *dev;
int memory_init(void) {
	
	int resault;
	resault = register_chrdev(memory_major, "memory", &memory_fops);
	if (resault < 0) {
		printk("cannot obtain major num: %d\n",
			memory_major);
		goto fail;
	}
	dev=(struct mem_dev *) kmalloc(sizeof(struct mem_dev), GFP_KERNEL); //实例化dev结构
	if (!dev) {
		resault = -ENOMEM;
		goto fail;
	}
	dev->data= kmalloc(1024, GFP_KERNEL);        //分配内存
	dev->size= 1024;
	dev->length= 0;
	init_MUTEX(&(dev->sem));                     //初始化旗表
	//init_MUTEX_LOCKED(&(dev->sem));
	if (!dev->data) {
		resault = -ENOMEM;
		goto fail;
	}
	memset(dev->data, 0, 1024);
	printk("<1>memory malloc succeed\n");
	
	return 0;
	fail:
		memory_exit();
		return resault;
}
	


int memory_open(struct inode *inode, struct file *file) {
	file->private_data=dev;
	return 0;
}



void memory_exit(void) {
	unregister_chrdev(memory_major, "memory");
	if(dev->data) {
		kfree(dev->data);
	}
	if(dev) {
		kfree(dev);
	}
	printk("<1>Removing memory device\n");
}


ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	struct mem_dev *dev;
	ssize_t ret=0;
	dev=filp->private_data;
	if( down_interruptible(&dev->sem))            //上锁
		return -ERESTARTSYS;
	
	if(dev->length<*f_pos) {
		return -EFAULT;
	}
	if((dev->length-*f_pos)<count ){
		count=dev->length-*f_pos;
	}
	if(copy_to_user(buf,dev->data+*f_pos,count)) {
		ret=-EFAULT;
		goto out;
	}

	if(copy_to_user(buf,dev->data+*f_pos,count)) {
		ret=-EFAULT;
		goto out;
	}
	*f_pos+=count;
	ret=count;
	out:
		up(&dev->sem);
		return ret;
	
}


ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *fpos) {
	ssize_t ret=0;
	struct mem_dev *dev;
	dev=filp->private_data;
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if(count>dev->size){
		count = dev->size;
	}
	memset(dev->data,0,dev->size);
	dev->length=0;
	if(copy_from_user(dev->data,buf,count)) {
		ret = -EFAULT;
		goto out;
	}
	dev->length+=count;
	*fpos+=count;
	ret=count;
	out:
		up(&dev->sem);
		return ret;
}

int memory_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err=0;
	int ret=0;
	struct mem_dev *dev;
	
	struct msg *m;
	dev=filp->private_data;
	m=(struct msg *) arg;
	int count=m->size;
	/*if( down_interruptible(&dev->sem))
	*	return -ERESTARTSYS;
	*/
	if(_IOC_TYPE(cmd) !=MEMORY_IOC_MAGIC)
		return -ENOTTY;                  //魔数非法
	if(_IOC_NR(cmd) > MEM_MAX)
		return -ENOTTY;                  //没有对应的命令
	if(_IOC_DIR(cmd) & _IOC_READ)        //地址合法性判断
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;
	
	if( down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	switch (cmd)
	{
		case MEM_CLEAR :
			memset( dev->data,0,dev->size);
			dev->length=0;
			break;
		case MEM_SET_STR :
			if(count>dev->size){
				count = dev->size;
			}
			memset(dev->data,0,dev->size);
			dev->length=0;
			if(copy_from_user(dev->data,(char *)m->data,count)) {
				ret = -EFAULT;
				goto out;
			}
			dev->length=m->size;
			printk("<1>__get_user%s",(char *)m->data);
			
			break;
	}
	out:
		up(&(dev->sem));
		return ret;
}

