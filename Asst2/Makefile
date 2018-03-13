CC = gcc
CFLAGS = -g -c 
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a
util: page_util.h
	$(CC) $(CFLAGS) util.c
my_pthread.a: my_pthread.o 
	$(AR) libmy_pthread.a page_alloc.o util.o my_pthread.o 
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h page_alloc.o util 
	$(CC) -pthread $(CFLAGS) my_pthread.c 

page_alloc.o:
	$(CC) $(CFLAGS) page_alloc.c

clean:
	rm -rf testfile *.o *.a tests/*
