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
#define MAX_INODES 23000
//#define WRITE_ZONE (sizeof(dummyInode)*MAX_INODES)/BLOCK_SIZE  + (128 * MAX_INODES)/BLOCK_SIZE
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
   //Destory was called 
   if(init == 512 && *(int*)buffer == 0)
   {
        init = 0;
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
   FT->files = malloc(FT->size*sizeof(Inode*));
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
        file->linkcount = 0;
        file->spaceleft = BLOCK_SIZE;
        file->next = -1;
        file->prev = -1;
        file->is_init = false;
        file->parent = -1;
        blockCount+=1;
        if(blockCount == (int)(BLOCK_SIZE/sizeof(struct dummyInode)))
        {
            blockCount = 0;
            blockCurr++;
        }

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
        file->file_mode = temp->file_mode;
        file->is_init = temp->is_init;
        file->linkcount = temp->linkcount;
        file->time_m = temp->time_m;
        file->time_a = temp->time_a;
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
    }
    else if(blockCount == 4)
    {
            blockCount = 0;
            blockCurr++;

    }
    blockCount+=1;

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
   FT->write_zone = blockCurr+1;
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
	int i;
	for(i=0;i<FT->size;i++){
		if(FT->files[i]->fd == fd){
			return FT->files[i];
		}

	}
	return NULL;
   /* while(pos < FT->size && ptr->fd != fd)
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
    }*/
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
            FT->num_free_inodes-=1;
			FT->files[i]->is_init =  1;
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
                    if(i < 0)
                    {
                        if(ptr->parent == 0)
                        {
                            return 1;
                        }
                        else
                        {
                            return 0;
                        }
                    }
					continue;
			}
            firstSlash = i;
            memcpy(fileName,(path+firstSlash+1),secondSlash-firstSlash-1);
            fileName[secondSlash-firstSlash-1]='\0';
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
    //sleep(15);
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
    char block[128];
    writeFS(0);
    struct sfs_state * sfs_data = (struct sfs_state *)userdata;
    disk_close(sfs_data->diskfile);
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
    	//statbuf->st_mode = file->file_mode;
        statbuf->st_mode = file->file_mode;
    	statbuf->st_nlink = file->linkcount;
	}
    statbuf->st_uid = getuid();
    statbuf->st_gid = getgid();
    statbuf->st_size = fileSize(file);
    statbuf->st_atime = file->time_a;
    statbuf->st_mtime = file->time_m;
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
    Inode * file = findFreeInode();
    if(file == NULL)
    {
        return -ENOMEM;
    }
    file->is_init = 1;
	char* buf = malloc(strlen(path));
	strcpy(buf,path);
	char * last = strrchr(buf, '/');;
	//problem for future Sam and SaraAnn - what if last is null?
	last = last+1;
	if(last !=NULL)
	{
		strcpy(file->fileName,last);
	}

    //memcpy(file->fileName,path,strlen(path));
    file->file_mode = S_IFREG | 0755;
    file->timestamp = time(NULL);
    file->time_m = time(NULL);
    file->time_a = time(NULL);
    //right now this just hardcodes to set root to be the parent
	//TODO: use SaraAnn's string parsing method to get the actual parent
	file->parent =  get_parent(path);
    file->permissions = O_RDWR;
    writeFS(0);
    return retstat;
}

int get_parent (const char * full_path)
{
	char * buffer = malloc(128);
	memcpy (buffer, full_path, 128);
	//if you've fucked up royally and somehow got passed the root directory as a param
	if (full_path == NULL || strlen(full_path) == 1)
		return -5;
	int i;
	char curr;
	for (i = strlen(full_path)-1; curr!= '/'; i--)
	{
		curr = full_path[i];
	}
	//When crr == '/' we go back i again, so we need to bring it back
	i++;	
	//return root if the param was directly below root directory
	if (i == 0)
	{
		return 0;
	}
	//truncate the path so that it just leads to the parent directory
	else
	{
		buffer[i] = '\0';
		return getFilePath(buffer)->fd;
	}
}

int reinit(Inode * victim)
{
	victim->fileName[0] = '\0';
	victim->is_init = 0;
	victim->parent = -1;
	victim->next = -1;
	victim->prev = -1;
	victim->spaceleft = BLOCK_SIZE;
    victim->file_type = 0;
    victim->file_mode = 0;
    return 0;
}


//In a rush: method zeros out the block corresponding with this inode
void cleanSpace(Inode * victim)
{
    int fd = victim->fd;
    char * emptyBlock = calloc(1,512);
    int jump = fd/512;
    int ret = block_write(FT->write_zone+jump,emptyBlock);
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);
	char * buffer = malloc(128);
	strcpy(buffer,path);
	int next;
	Inode * victim = getFilePath(buffer);
	while(victim != NULL)
	{
		next = victim->next;
		int ret = reinit(victim);
		if(ret != 0)
		{
			log_msg("Something wrong with unlink");
		}
        cleanSpace(victim);
        FT->num_free_inodes++;
		victim = getFileFD(next);
	}
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
//Do we have to conside the user/group/all permissions when opening a file?
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);
    char * buffer = malloc(128);
    strcpy(buffer,path);
    Inode * file = getFilePath(buffer);
    file->time_a = time(NULL);
    if(file == NULL)
    {
        errno = ENOENT;
        return -1 * errno;
    }
    if(file->file_type == S_IFDIR)
    {
        errno = EISDIR;
        return -1 * errno;
    }
    file->permissions = O_RDWR;
    writeFS(0);
    return 0;
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
//TODO: see about padding with zeros - it seems like the only time would need to do that is on EOF, no?
/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  TODO: wtf does this mean???
 * An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    int num_read = 0;
	int total_read = 0;
	log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
	char * path_buffer = malloc(128);
	strcpy(path_buffer, path);
	Inode * file = getFilePath(path_buffer);
    file->time_a = time(NULL);
	if (file->permissions != O_RDONLY && file->permissions != O_RDWR)
	{
		errno = EACCES;
		return -1;
	}
	//if the offset is bigger than a block, move along inodes until you find the correct block
	while (offset >= BLOCK_SIZE)
	{
		file = getFileFD(file->next);
		offset -= BLOCK_SIZE;
	}
	int jump = file->fd/512;
  	char * intermediate_buffer = malloc(BLOCK_SIZE);	
	
	if (offset > 0)
	{
		int ret = block_read(FT->write_zone+jump, intermediate_buffer);
		//copy only the relevant bytes into the buffer being returned to the user
		memcpy(buf, intermediate_buffer+offset, 512-offset); 
		total_read = 512 - offset;
		size -= total_read;
		//progress to the next block
		file = getFileFD(file->next);
		jump = file->fd/BLOCK_SIZE;
	}
	while (size > 0)
	{
		int ret = block_read(FT->write_zone+jump, intermediate_buffer);
		if (size >= 512)
		{
			memcpy(buf+total_read, intermediate_buffer, 512);
			num_read = 512;
			total_read += 512;
		}
		else
		{
			memcpy(buf+total_read, intermediate_buffer, size);
			num_read = size;
			total_read += num_read;
		}
		size -= num_read;
		file = getFileFD(file->next);
        if(file == NULL)
        {
            break;
        }
		jump = file -> fd/BLOCK_SIZE;
	}
    return total_read;
}

//TODO: if there isn't enough space to write all that was requested, should you write as much as there IS space for, and return that, or should you just reject the whole operation?
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
    //# of bytes to originally write, since size variable changes
	int the_size = size;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    char * buffer = malloc(strlen(path));
    strcpy(buffer,path);
    Inode * file = getFilePath(buffer);
    file->time_m = time(NULL);
    file->time_a = time(NULL);
	//check and validate write permissions
    if(file->permissions != O_WRONLY && file->permissions != O_RDWR)
    {
        errno = EACCES;
        return -1;
    }
    //this math isn't quite perfect - for example if the offset is 512 but the size is 1, it should get two inodes but here it only gets one
    //We shouldn't link inodes if we know there aren't enough free files for this to work
        //this loop is for allocation of ENTIRE blocks if there isn't already sufficient space linked to the original inode
    int testOffset = offset;
    Inode * temp = file;
    while(temp->next != -1)
    {
        testOffset -= BLOCK_SIZE;
        temp = getFileFD(temp->next);
    }
	if((int)(size/BLOCK_SIZE) + (int)(testOffset/BLOCK_SIZE) > FT->num_free_inodes)
    {
        errno = ENOMEM;
        return -1;
    }
    while(offset >= BLOCK_SIZE)
    {
        Inode * old = file;
        file = getFileFD(file->next);
        if(file == NULL)
        {
            Inode * newFile = findFreeInode();
            //THIS SHOULD NOT HAPPEN. IF YOU GET HERE AND THIS HAPPENS SOMETHING IS HORRIBLY WRONG
			if(newFile == NULL)
            {
                errno = ENOMEM;
                return -1;
            }
            old->next = newFile->fd;
            newFile->prev = old->fd;
            file = newFile;
        }
    	offset -= BLOCK_SIZE;
    }
   

    free(buffer);
    buffer = calloc(1,512);
	//#of bytes left to write in CURRENT (not necessarily starting) BLOCK
	int amountLeft = BLOCK_SIZE - offset;
    int jump = file->fd / 512;
    int currPointer = 0;
    //this is for the actual writing process
	while(size > 0)
    {
        int orgSize = size;
        if(size < amountLeft)
        {
            size = 0;
        }
        else
        {
            size -= amountLeft;
        }
        //When deciding how many bytes to memcpy from the user buffer
        //we look at which one is smaller, size or amountLeft
        //The smaller one should be the amount of bytes we copy over
        	//this is the case you don't write from the very beginning, in which case you need to read, rewrite the old info, and then rewrite whatever is left
		if(offset != 0)
        {
            int ret = block_read(FT->write_zone + jump, buffer);
            if(orgSize < amountLeft)
            {
                memcpy(buffer+offset,buf+currPointer,orgSize);
                currPointer += orgSize;
                file->spaceleft -= orgSize;
            }
            else
            {
                memcpy(buffer+offset,buf+currPointer,amountLeft);
                currPointer += amountLeft;
                file->spaceleft-=amountLeft;
            }
            offset = 0;
        }

        else if(orgSize < amountLeft)
        {
            if(orgSize < BLOCK_SIZE)
            {
                int ret = block_read(FT->write_zone + jump, buffer);
            }
            memcpy(buffer,buf+currPointer,orgSize);
            currPointer += orgSize;
            file->spaceleft-=orgSize;
        }
        else
        {
            if(orgSize < BLOCK_SIZE)
            {
                int ret = block_read(FT->write_zone + jump, buffer);
            }
            memcpy(buffer,buf+currPointer,amountLeft);
            currPointer += amountLeft;
            file->spaceleft-=amountLeft;
        }
        int ret = block_write(FT->write_zone +jump,buffer);
        if(ret == 0 || ret < 0)
        {
            log_msg("I/O ERROR Hello?\n");
            return -1;
        }
        if(size > 0)
        {
            Inode * oldNext = file;
            file = getFileFD(file->next);
            if(file == NULL)
            {
                Inode * newFile = findFreeInode();
                if(newFile == NULL)
                {
                    log_msg("This Error should not occur, happening in WRITE\n");
                    return -1;
                }
                oldNext->next = newFile->fd;
                newFile->prev = oldNext->fd;
                file = newFile;
            }
            jump = file->fd/BLOCK_SIZE;
        }
        amountLeft = BLOCK_SIZE;
        
    }
    return the_size;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
   
    Inode * dir = findFreeInode();

    if(strlen(path) == 1)
    {
        //Root Directory???!!!????
        return -99;
    }
    if(dir == NULL)
    {
        errno = -ENOMEM;
        return -1;
    }
    dir->is_init = true;
    dir->file_type = S_IFDIR;
    dir->file_mode = S_IFDIR | 0755;
	//TODO: do we increment linkcount if a file is created in the directory?
	dir->linkcount = 2;
    dir->timestamp = time(NULL);
    dir->time_m = time(NULL);
    dir->time_a = time(NULL);
    dir->parent = get_parent(path);
    int slash = strlen(path)-1;
    while(path[slash] != '/')
    {
        slash--;
    }
    strcpy(dir->fileName,path+slash+1);
    writeFS(0);
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    char * buffer = malloc(128);
    strcpy(buffer,path);
    Inode * dir = getFilePath(buffer);
    //Check if the Dir is empty
    int i = 1;
    while( i < FT->size)
    {
        if(FT->files[i]->parent == dir->fd)
        {
            errno = ENOTEMPTY;
            return -1;
        }
        i++;
    }
    //Directory is empty, commence the uninitialization!
    reinit(dir);
    writeFS(0);
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
    char * buffer = malloc(512);
    strcpy(buffer,path);
    Inode * dir = getFilePath(buffer);
    free(buffer);
    dir->time_a = time(NULL);
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
    dir->time_a = time(NULL);
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

//WTF IS THIS
/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    return retstat;
}

int sfs_truncate(const char* path,off_t size){
	return 0;

}
struct fuse_operations sfs_oper = 
{
  .init = sfs_init,
  .destroy = sfs_destroy,
  .truncate = sfs_truncate,
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
