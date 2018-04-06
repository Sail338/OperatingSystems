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

//Loaf the File System from Disk
int loadFS()
{
   char buffer[BLOCK_SIZE];
   int init = block_read(0,buffer);
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
        FT->num_free_inodes = (int)buffer[1];
        FT->size = (int)buffer[5];
        ret = block_read(BLOCK_SIZE,buffer);
   }
   FT->files = malloc(FT->num_free_inodes*sizeof(Inode*));
   int i;
   int blockCount = 0;
   int blockCurr = 1;
   for(i=0; FT->size;i++)
   {
    Inode * file = FT->files[i];
    file=malloc(sizeof(Inode));
    file -> fd = i*BLOCK_SIZE;
    if(init == 0)
    {
        file->permissions = -1;
        file->file_type = 0;
        file->spaceleft = BLOCK_SIZE;
        file->next = NULL;
        file->prev = NULL;
        file->is_init = false;
    }
    else
    {
        Inode * temp = (struct dummyInode)buffer[blockCount*sizeof(struct dummyInode)];
        file->permissions = temp->permissions;
        file->file_type = temp->file_type;
        file->spaceleft = temp->spaceleft;
        file->next = temp-> is_init = temp->is_init
    
        blockCount+=1;
        if(blockCount == (int)(BLOCK_SIZE/sizeof(struct dummyInode))+1)
        {
            blockCount = 0;
            blockCurr++;
            ret = block_read(BLOCK_SIZE*blockCurr,buffer);
            if(ret == 0 || ret < 0)
            {
                return -99;
            }
        }
    }
   }
   //blockCurr doesnt increment on the last one to move on to the file paths so we increment after the for loop
   blockCurr++;
   ret = block_read(BLOCK_SIZE*blockCurr,buffer);
   //Going through path blocks now
   for(i = 0; FT->size;i++)
   {
    Inode * file = FT->file[i];
    if(init != 0)
    {
        mempcy(file->path,buffer,BLOCK_SIZE);
        blockCurr++;
        ret = block_read(BLOCK_SIZE*blockCurr,buffer);
        if(ret == 0 || ret < 0)
        {
            return -99;
        }
    }
   }
}





Inode * getFileFD(int fd)
{
    if(IS_FILE_TABLE_INIT == 0)
    {
        //This method should not be called if the File Table is not created
        return NULL;
    }
    Inode * ptr = FT->files[0];
    int pos = 1;
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

Inode * getFilePath(char * path)
{
    
    if(path == NULL)
    {
        log_msg("Nullerino");
        return NULL;
    }
    Inode * ptr = FT->files[0];
    int pos = 1;
    while(pos < FT->size && strcmp(path,ptr->path) != 0)
    {
        if(FT->files[pos]->path == NULL)
        {
            pos+=1;
            continue;
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
    file = file->next;
    while(file != NULL)
    {
        total += (BLOCK_SIZE - file->spaceleft);
        file = file->next;
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
        file = file->next;
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
    disk_open((SFS_DATA)->diskfile);
	FT = malloc(sizeof(FileTable *));
	FT ->num_free_inodes = totalsize/BLOCK_SIZE;
	FT->size = totalsize/BLOCK_SIZE;
	FT ->files = malloc(FT->num_free_inodes*sizeof(FileTable *));
	int i = 0;

	for(i=0;i<FT->num_free_inodes;i++)
    {
		FT->files[i] = malloc(sizeof(Inode));
		FT ->files[i] -> file_position = 0;
		FT ->files[i]->fd = i*BLOCK_SIZE;
		FT->files[i]->permissions = -1;
		FT -> files[i] ->file_type = NONE;
		FT -> files[i] -> spaceleft = BLOCK_SIZE;
		FT -> files[i] -> next = NULL;
		FT ->files[i] ->prev = NULL;
		FT ->files[i]->is_init = false;
		FT->files[i] -> path = NULL;
        FT->files[i]->linkcount = 0;
        FT->files[i]->timestamp = 0;
    }	
        FT->files[0]->path = malloc(2);
        strcpy(FT->files[0]->path,"/");
		FT->files[0]->is_init = true;
		FT->files[0] -> file_type = DIR_NODE;

		
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
    if(getFilePath(fpath) == NULL)
    {
        errno = EBADF;
        return -1;
    }
   	//lookup the FILE TABLE FOR THE PATH;
    statbuf->st_dev = 0;
    statbuf->st_ino = 0;
    Inode * file = getFilePath(fpath);
	if(strcmp(path, "/")==0){
	
    	statbuf->st_mode = S_IFDIR | 0755;
   	 	statbuf->st_nlink = 2;
	}
	else{
    	statbuf->st_mode = S_IRWXU;
    	statbuf->st_nlink = file->linkcount;
	}
    log_msg("HERE!\n");
    //How do we get the userid of the person who ran the program?
    statbuf->st_uid = getuid();
    statbuf->st_gid = getgid();
    log_msg("HERE2\n");
    statbuf->st_size = fileSize(file);
    log_msg("HERE3\n");
    statbuf->st_atime = file-> timestamp;
    log_msg("HERE4\n");
    statbuf->st_mtime = 0;
    statbuf->st_ctime = 0;
    statbuf->st_blksize = 0;
    statbuf->st_blocks = fileTotalSize(file)/BLOCK_SIZE;

	log_msg("HERE5\n");
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
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);
    
    
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
