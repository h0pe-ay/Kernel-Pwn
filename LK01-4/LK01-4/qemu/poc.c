#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
int success = 0;
void *thread_function(void *arg) {
	while(1)
	{
		while (!success)
		{
			int fd = open("/dev/holstein",O_RDWR);
			if (fd == 4)
				success = 1;
			if (fd != -1 && success == 0)
				close(fd);
		}
		if (write(3, "a", 1) != 1 || write(4, "a", 1) != 1)
		{
			close(3);
			close(4);
			success = 0;
		} 
		else
			break;
	}
	
}

int main()
{
	pthread_t thread_id1, thread_id2;
	if (pthread_create(&thread_id1, NULL, thread_function, NULL) != 0)
	{
		fprintf(stderr, "thread error\n");
		return 1;
	}
	if (pthread_create(&thread_id2, NULL, thread_function, NULL) != 0)
	{
		fprintf(stderr, "thread error\n");
		return 1;
	}
	pthread_join(thread_id1, NULL);
	pthread_join(thread_id2, NULL);	
	char temp[0x20]= {};
	write(3, "abcdefg", 7);
	read(4, temp, 7);
	if (strcmp(temp, "abcdefg"))
	{
		puts("fail\n");
		exit(-1);
	}
	printf("sucess\n");
}
