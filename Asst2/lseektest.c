#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
int main()

{
        int fd = open("swapfile", O_RDWR|O_CREAT,0777);
		lseek(fd,1000,SEEK_SET);	
		perror("Error in Write");
        write(fd,"0",1);
        return 1;
		char gb[1000];
		strcpy(gb,"Hey");
		*(int*)(gb+5) = 4;
	    lseek(fd,0,SEEK_SET);
   		write(fd,gb,30);
	 	char grab[4096];
		lseek(fd,0,SEEK_SET);
		read(fd, (void*)grab,30);
		printf("The String: %s\n",grab);
		printf("The Int: %d\n",*(int*)(grab+5));	
        close(fd);	
		
}
