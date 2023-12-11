#include <inc/lib.h>


void _main(void)
{

	int i1=0;
	char buff1[256];
	atomic_readline("Please enter a number:", buff1);
	i1 = strtol(buff1, NULL, 10);

	int f=4*1042;

	for(int i=0;i<i1;i++){
	 int *x =malloc(f);
	 *x=4;
	}

	atomic_cprintf("\nshehap finish\n");


	return;
}

