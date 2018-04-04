#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
int IS_FILE_TABLE_INIT;
#define totalsize 16777216
#define NONE 0
#define FILE_NODE 1
#define DIR_NODE 2
typedef struct Inode
{
			//we only care aboyt the positon for the first inode in the chain
				int file_position;
				int fd;
				//permissions the file was created with
				int permissions;
						//permssions the file currently using
				int  file_mode;
			//is this a file or folder
				char  file_type;
	
				//path
				char *path;
				//timestamp
				time_t timestamp;
				//above is what we only care about in the frist thing in the chain
				//space left in INODE
				int spaceleft;
				//pointer to next inode in the chain
			    struct Inode *  next;
				struct Inode *prev;
				bool is_init;
				//int linkcount;
			
}Inode;
			
			
			
			
/*
	*table that maps Inodes and stutff
*/
typedef struct FileTable
{
	Inode ** files;
	int num_free_inodes;
	int size;			
			
}FileTable;
FileTable * FT;
