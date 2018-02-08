//File: my_pthread_t.h
//Author: Yujie REn
//Date: 09/23/2017

//name: SaraAnn Stanway, Srihari Shankar, Sam Azouzi
//username of iLab: sys41
//iLab server: template

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include "util.h"
#include <ucontext.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef threadNode my_pthread_t;

typedef struct threadControlBlock {
	/* add something here */
} tcb; 


// Feel free to add your own auxiliary data structures


/* Function Declarations: */
void normal_sig_handler(int);

void yeild_sig_handler(int);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif

