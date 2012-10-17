#include <asm/ioctl.h>
#include <linux/ioctl.h>
#include <linux/fs.h>


#define MEMORY_IOC_MAGIC 'k'

#define MEM_CLEAR     _IO(MEMORY_IOC_MAGIC, 1)   //0号命令，将整个内存区清零
#define MEM_SET_STR   _IOW(MEMORY_IOC_MAGIC,2,int) //1号命令，写入字符串

#define MEM_MAX 4

struct msg {
	void *data;
	int size;
};
	
