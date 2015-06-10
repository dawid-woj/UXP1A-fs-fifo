#ifndef FIFO_MUTEX_H_
#define FIFO_MUTEX_H_
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define LINK 1
#define UNLINK 2
#define TOKEN 3
#define ERROR 4
#define UNMOUNT 5
#define UNMOUNT_PREPARE 6
#define UNMOUNT_EXECUTE 7
#define NO_WRITERS 8

struct fifo_msg
{
  int type;
  int code;
  int mpid;
};


struct proc_data
{
	int wrfifo_fd;
	int myfifo_fd;
	char *myfifo_name;
	int nextproc_pid;
};




int fifomutex_lock();
int fifomutex_unlock();

void no_writers_error(struct proc_data *data, int mypid);
void add_new_proc(struct proc_data *data, struct fifo_msg *msg);
void fifo_unlink(int mypid, struct proc_data *data, struct fifo_msg *msg);
void clear_fifos(struct proc_data *data);
void initialize(struct proc_data *data, int mypid);


int fifomutex_startinit();
int fifomutex_umount();
#endif


