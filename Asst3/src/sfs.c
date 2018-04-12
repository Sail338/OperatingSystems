/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.

*/

#include "params.h"
#include "block.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif
#define MAX_INODES 1530
#include "log.h"
#include "util.h"
///////////////////////////////////////////////////////////////////////
//
//Helper Function
//

int ceil_bytes(int numBytes)
{
    double blk_size = BLOCK_SIZE;
    double bytes = numBytes;
    double divided = bytes/blk_size;
    int x = (int)divided;
    if(divided - (double)x > 0)
    {
        return x+1;
    }
    return x;
}
//If init is 1 then we cannot continue any file, but if it is 0 we can skip some because we know that there is
//def. a node in that place.
//If 0 is returned -> everything went accordingly
int writeFS(int init)
{
    char * buffer = malloc(512);
    buffer[0] = 1;
    *(int*)(buffer+1) = FT->num_free_inodes;
    *(int*)(buffer+5) = FT->size;
    int ret = block_write(0,buffer);
    int blockCurr = 1;
    int blockCount = 0;
    int i;
    int retStruct = block_read(blockCurr, buffer);
    for(i = 0; i < FT->size; i++)
    {
        Inode * file = FT->files[i];
       /* if(file->modified == 0)
        {
            blockCount += 1;
            if(blockCount == (int)(BLOCK_SIZE/sizeof(struct dummyInode)) + 1)
            {
				block_write(blockCurr,buffer);
                blockCount = 0;
                blockCurr++;
                retStruct = block_read(blockCurr,buffer);
            }
            continue;
        }*/
        dummyInode * temp = malloc(sizeof(dummyInode));
        temp->file_position = file->file_position;
        temp->fd = file->fd;
        temp->permissions = file->permissions;
        temp->file_mode = file->file_mode;
        temp->file_type = file->file_type;
        temp->timestamp = file->timestamp;
        temp->spaceleft = file->spaceleft;
        temp->next = file->next;
        temp->prev = file->prev;
        temp->parent = file->parent;
        temp->is_init = file->is_init;
        temp->linkcount = file->linkcount;
        if(blockCount == (int)(BLOCK_SIZE/sizeof(struct dummyInode)))
        {
            blockCount = 0;
            int ret = block_write(blockCurr,buffer);
            if(ret < 0)
            {
                return ret;

            }
            blockCurr++;
           retStruct = block_read(blockCurr, buffer);
        }
        memcpy(buffer+(blockCount*sizeof(dummyInode)),temp,sizeof(dummyInode));
        blockCount++;
    }
    if(blockCount != 0)
    {
        int ret = block_write(blockCurr,buffer);

        if(ret < 0)
        {
            return ret;
        }
        blockCurr++; 
    }
    blockCount = 0;
	int j;
    char readbuffer[BLOCK_SIZE];
    int retread = block_read(blockCurr, readbuffer);
    for(j =0; j < FT->size;j++)
    {
        Inode * file = FT->files[j];
/*        if(file->modified ==0 && init == 0)
        {
            blockCount++;
         *   if(blockCount == 4)
            {

            	int ret = block_write(blockCurr,readbuffer);
                blockCount = 0;
                blockCurr++;
                retread = block_read(blockCurr, readbuffer);

            }
            continue;*/
        
        memcpy(readbuffer+(blockCount * (BLOCK_SIZE/4)),file->fileName,(int)(BLOCK_SIZE/4));
        blockCount++;
        if(blockCount == 4)
        {
            blockCount = 0;
            int ret = block_write(blockCurr,readbuffer);
            blockCurr++;
            if(ret < 0)
            {
                return ret;
            }
            retread = block_read(blockCurr, readbuffer);
        }
        file->modified = 0;
    }
    if(blockCount != 0)
    {
        int ret = block_write(blockCurr,readbuffer);
        if(ret < 0)
        {
            return ret;
        }
    }
    return 0;
}







//Load the File System from Disk
//If this function returns 0 -> assume all went well and you have the data structure in memory
//If this function returns -99 -> this shouldnt be happening
//If this function returns <0 && !-99 -> systems error
int loadFS()
{
   char buffer[BLOCK_SIZE];
   int init = block_read(0,buffer);
   int ret = 0;
   if(init < 0)
   {
        return init;
   }
   FT = malloc(sizeof(FileTable*));
   if(init == 0)
   {
        FT->num_free_inodes = MAX_INODES;
        FT->size = MAX_INODES;
   }
   else
   {
        FT->num_free_inodes = *(int*)(buffer+1);
        FT->size = *(int*)(buffer+5);
        ret = block_read(1,buffer);
        if(ret < 0)
        {
            return ret;
        }
   }
   FT->files = malloc(FT->num_free_inodes*sizeof(Inode*));
   int i;
   int blockCount = 0;
   int blockCurr = 1;
   for(i=0; i<FT->size;i++)
   {
    FT->files[i]=(Inode *)malloc(sizeof(Inode));
    Inode * file = FT->files[i];
    file -> fd = i*BLOCK_SIZE;
    //file->modified=1;
    if(init == 0)
    {
        file->permissions = -1;
        file->file_type = 0;
        file->file_mode = 0;
        file->timestamp = 0;
        file->file_position = 0;
        file->linkcount = 0;
        file->spaceleft = BLOCK_SIZE;
        file->next = -1;
        file->prev = -1;
        file->is_init = false;
        file->parent = -1;
    }
    else
    {
        dummyInode * temp = (struct dummyInode*)(buffer+(blockCount*sizeof(struct dummyInode)));
        file->permissions = temp->permissions;
        file->file_type = temp->file_type;
        file->spaceleft = temp->spaceleft;
        file->next = temp-> next;
        file->prev = temp->prev;
        file->parent = temp->parent;
        file->timestamp = temp->timestamp;
        file->file_position = temp->file_position;
        file->file_mode = temp->file_mode;
        file->is_init = temp->is_init;
        file->linkcount = temp->linkcount;
        blockCount+=1;
        if(blockCount == (int)(BLOCK_SIZE/sizeof(struct dummyInode)))
        {
            blockCount = 0;
            blockCurr++;
            ret = block_read(blockCurr,buffer);
            if(ret == 0 || ret < 0)
            {
                return -99;
            }
        }
    }
   }
   //blockCurr doesnt increment on the last one to move on to the file paths so we increment after the for loop
   if(blockCount != 0)
   {
        blockCurr++;
   }
   ret = block_read(blockCurr,buffer);
   blockCount = 0;
   //Going through file name blocks now
   for(i = 0; i < FT->size;i++)
   {
    Inode * file = FT->files[i];
    if(init != 0)
    {
        if(blockCount == 4)
        {
            blockCount = 0;
            blockCurr++;
            ret = block_read(blockCurr,buffer);
            if(ret ==0 || ret < 0)
            {
                log_msg("NUMA 2        %d\n",ret);
                return -99;
            }
			//continue;

        }
        memcpy(file->fileName,buffer+(blockCount*128),BLOCK_SIZE/4);
        blockCount += 1;
    }
   }
   if(init == 0) 
   {
        int writeRet = writeFS(1);
        fprintf(stderr,"Senpai notice me my dude\n");
        if(writeRet < 0)
        {
            return writeRet;
        }
   }
   return 0;
}





Inode * getFileFD(int fd)
{
/*    if(IS_FILE_TABLE_INIT == 0)
    {
        //This method should not be called if the File Table is not created
        return NULL;
    }*/
    Inode * ptr = FT->files[0];
    int pos = 0;
    while(pos < FT->size && ptr->fd != fd)
    {
        ptr = FT->files[pos];
        pos += 1;
    }
    //Could not find referenced fd
    if(pos >= FT->size)
    {
        return NULL;
    }
    else
    {
        return ptr;
    }
}


Inode * findFreeInode()
{
    int i = 0;
    while(i < FT->size)
    {
        if(FT->files[i]->is_init == 1)
        {
            i++;
        }
        else
        {
            return FT->files[i];
        }
    }
    return NULL;
}


int validatePath(char * path, Inode * ptr)
{
    int firstSlash = 0;
    int secondSlash = strlen(path)-1;
    int i = strlen(path)-1;
    char fileName[128];
	int init =0;
    while(i >= 0)
    {
        if(path[i] == '/')
        {
			if(init == 0)
			{
					secondSlash=i;
					init = 1;
					i--;
					continue;
			}
            firstSlash = i;
            memcpy(fileName,(path+firstSlash+1),secondSlash-firstSlash+1);
            secondSlash=i;
            if(strcmp(getFileFD(ptr->parent)->fileName,fileName) == 0)
            {
                ptr = getFileFD(ptr->parent);
            }
            else
            {
                return 0;
            }
        }
        i--;
    }
    return 1;
}


Inode * getFilePath(char * path)
{
    if(path == NULL)
    {
        log_msg("Nullerino");
        return NULL;
    }
    int fileNameIndex = strlen(path)-1;
    while(path[fileNameIndex] != '/')
    {
        fileNameIndex--;
    }
    fileNameIndex++;
    if(fileNameIndex > strlen(path)-1)
    {
        return FT->files[0];
    }
    char fileName[128];
    strcpy(fileName,(path+fileNameIndex));
    Inode * ptr = FT->files[1];
    int pos = 2;
    while(pos < FT->size) 
    {
        if(ptr->fileName == NULL)
        {
            ptr = FT->files[pos];
            pos+=1;
            continue;
        }
        if(strcmp(fileName,ptr->fileName) == 0)
        {
				//not sure what this is doing 
            int ret = validatePath(path,ptr);
            if(ret == 1)
            {
                break;
            }
        }
        ptr = FT->files[pos];
        pos+=1;
    }
    //Could not find referenced path
    if(pos >= FT->size)
    {
        return NULL;
    }
    else
    {
        return ptr;
    }

}

int fileSize(Inode * file)
{
    int total = BLOCK_SIZE - file->spaceleft;
    file = getFileFD(file->next);
    while(file != NULL)
    {
        total += (BLOCK_SIZE - file->spaceleft);
        file = getFileFD(file->next);
    }
    return total;
}



int fileTotalSize(Inode * file)
{
    int total = 0;
    file = FT->files[file->fd/BLOCK_SIZE];
    while(file != NULL)
    {
        total += BLOCK_SIZE;
        file = getFileFD(file->next);
    }
    return total;
}

//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//


/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *sfs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "in bb-init\n");
    log_msg("\nsfs_init()\n");
    //struct sfs_state * state = SFS_DATA; 
	//log_msg("STATE BEFORE %p",state);
    //log_fuse_context(fuse_get_context());
    log_conn(conn);
	sleep(15);
    disk_open((SFS_DATA)->diskfile);
	int ret = loadFS();
    if(ret != 0)
    {
        log_msg("COULD NOT LOAD FS!\n");
        return;
    }
    memcpy(FT->files[0]->fileName,"/",2);
	FT->files[0]->is_init = true;
	FT->files[0] -> file_type = S_IFDIR;
    FT->files[0]->modified = 1;
    writeFS(0);
    log_msg("Finished SFS INIT\n");	
    fuse_get_context()->uid = getuid();
    fuse_get_context()->gid = getgid();
    fuse_get_context()->pid = getpid();
    return SFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    strcpy(fpath,path); 
    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
    Inode * file = getFilePath(fpath);
    if(file == NULL)
    {
        return -ENOENT;
    }
   	//lookup the FILE TABLE FOR THE PATH;
    statbuf->st_dev = 0;
    statbuf->st_ino = 0;
	if(strcmp(path, "/")==0){
	
    	statbuf->st_mode = S_IFDIR | 0755;
   	 	statbuf->st_nlink = 2;
	}
	else{
    	statbuf->st_mode = file->file_mode;
    	statbuf->st_nlink = file->linkcount;
	}
    //How do we get the userid of the person who ran the program?
    statbuf->st_uid = getuid();
    statbuf->st_gid = getgid();
    statbuf->st_size = fileSize(file);
    statbuf->st_atime = time(NULL);
    statbuf->st_mtime = time(NULL);
    statbuf->st_ctime = file->timestamp;
    statbuf->st_blksize = 0;
    statbuf->st_blocks = fileTotalSize(file)/BLOCK_SIZE;
    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	errno = 0;
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);
    char * temp = malloc(strlen(path));
    strcpy(temp,path);
	//why do we need this?
   /* if(getFilePath(temp) == 0)
    {
        return retstat;
    }*/
    Inode * file = findFreeInode();
    if(file == NULL)
    {
        return -ENOMEM;
    }
    file->is_init = 1;
    file->modified = 1;
	char* buf = malloc(strlen(path));
	strcpy(buf,path);
	char * last = strrchr(buf, '/');;
	last = last+1;
	if(last !=NULL){
		strcpy(file->fileName,last);

	}

    //memcpy(file->fileName,path,strlen(path));
    file->file_mode = mode;
    file->timestamp = time(NULL);
    file->parent = FT->files[0]->fd;
    writeFS(0);

    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    
    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);

    
    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    

    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);

   
    return retstat;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    
    
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
   
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    
    
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    char * temp = malloc(strlen(path));
    strcpy(temp,path);
    Inode * dir = getFilePath(temp);
    int i = 0;
    while(i < FT->size)
    {
        Inode * file = FT->files[i];
        if(file->parent != dir->fd)
        {
			i++;
			continue;
        }
        else
        {
            struct stat * newStat = malloc(sizeof(struct stat));
            newStat->st_mode = file->file_type | 0755;
            if(filler(buf,file->fileName,newStat,0) != 0)
            {
                return -ENOMEM;
            }
        }
		i++;
    }
    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;


    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,

  .getattr = sfs_getattr,
  .create = sfs_create,
  .unlink = sfs_unlink,
  .open = sfs_open,
  .release = sfs_release,
  .read = sfs_read,
  .write = sfs_write,

  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    sfs_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
