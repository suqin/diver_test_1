#include <stdio.h>
#include <string.h>
#include <fcntl.h>  /*O_ACCMODE*/
#include "ioct.h"
int main(int argc, char *argv[])
{
	struct msg m;
	char *s;
    int devfd;
    devfd = open("/dev/memory", O_WRONLY);
	if (devfd == -1) {
		printf("Can't open /dev/memory\n");
		return -1;
	}
    if(argc == 1)
	{
		ioctl(devfd, MEM_CLEAR,NULL);
	}
	else  {
		s=argv[1];
		m.data=(void *) s;
		m.size=strlen(s);
		printf("Write %s",s);
		ioctl(devfd, MEM_SET_STR, &m);
		close(devfd);
	}
	return 0;
}
