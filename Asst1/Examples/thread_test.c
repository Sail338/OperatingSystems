#include <sys/ucontext.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#define MEM 64000
//folloing http://nitish712.blogspot.com/2012/10/thread-library-using-context-switching.html
//define two structs
ucontext_t a;
ucontext_t Main;
/**
 * The function to run
 */
void fn1(){
	printf("Hello Sri ur a cool boi");		
	return;
}
int  main(){
	//get the current context store it in one of the pointer,s also intializes the poinwer
	getcontext(&Main);
	//initialize the thread to switch to using getContext
	getcontext(&a);
	//set first param to null if you dont want another thread to run automaitially after this one
	 a.uc_link=0;
	 //stack stuff
 	a.uc_stack.ss_sp=malloc(MEM);
 	a.uc_stack.ss_size=MEM;
	//not sure what this last param does
 	a.uc_stack.ss_flags=0;
	//make the context and pass in the function
	 makecontext(&a, (void*)&fn1, 0);
	 printf("Hello world");
	 
	swapcontext(&Main,&a);
	return 0;
}
