#include <sys/ucontext.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#define MEM 64000
ucontext_t a;
ucontext_t Main;
void fn1(){
	printf("Hello Sri ur a cool boi");		
	return;
}
int  main(){
	getcontext(&Main);
	getcontext(&a);
	 a.uc_link=0;
 	a.uc_stack.ss_sp=malloc(MEM);
 	a.uc_stack.ss_size=MEM;
 	a.uc_stack.ss_flags=0;
	 makecontext(&a, (void*)&fn1, 0);
	 printf("Hello world");
	swapcontext(&Main,&a);
	return 0;
}
