#ifndef FIFO_MUTEX_H_
#define FIFO_MUTEX_H_
#include <stdio.h>

#define LINK 
#define UNLINK 
#define TOKEN
#define ERROR 
#define UNMOUNT 

struct fifo_msg
{
	int type;		
	int code;	
};

int fifomutex_lock();

int fifomutex_unlock();

int fsinit();

#endif


