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
			//we only care about the positon for the first inode in the chain
				int file_position;
				int fd;
				//permissions the file was created with
				int permissions;
						//permssions the file currently using
				int  file_mode;
			//is this a file or folder
				int  file_type;
	
				//file name for current file; Should be able to store 4 in one disk block
				char fileName[128];
				//timestamp
				time_t timestamp;
				//above is what we only care about in the frist thing in the chain
				//space left in INODE
				short spaceleft;
				//pointer to next inode in the chain
			    int next;
				int prev;
                //Used to build the path
                int parent;
				bool is_init;
				short linkcount;
                int modified;
			
}Inode;

typedef struct dummyInode
{

		        //we only care aboyt the positon for the first inode in the chain
				int file_position;
				int fd;
                int modified;
				//permissions the file was created with
				int permissions;
				//permssions the file currently using
				int  file_mode;
			    //is this a file or folder
				int  file_type;	
				//path
				//timestamp
				time_t timestamp;
				//above is what we only care about in the frist thing in the chain
				//space left in INODE
				short spaceleft;
				//fd to next inode in the chain
			    int next;
				int prev;
                //We can use this to build a complete path
                int parent;
				bool is_init;
				short linkcount;

}dummyInode;





			
			
/*
	*table that maps Inodes and stutff
*/
typedef struct FileTable
{
	Inode ** files;
	int num_free_inodes;
	int size;			
    int write_zone;
			
}FileTable;
FileTable * FT;
Inode * getFileFD(int);
Inode * getFilePath(char*);
int fileSize(Inode *);
int loadFS();
int ceil_bytes(int);
int writeFS(int);
int get_parent(const char *);
int reinit(Inode *);
