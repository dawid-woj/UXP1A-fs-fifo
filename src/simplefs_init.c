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
#include "fifo_mutex.h"

char *initfifo_name = "initfifo";

int fifomutex_init();

int secondproc_pid=-1;
int initfifo_fd;
	
int unmount_state=0;		
	
struct fifo_msg msg;
int wrfifo_fd=-1;
char *wr_fifoname;
int wr_pid = -1;


void ino_writers_error();
void u_linkreq();
void iadd_new_proc();
void start_unmount();
void no_writers_msg();

int main()
{
	signal(SIGPIPE, SIG_IGN);
	fifomutex_init();
	return 0;
}



int fifomutex_init()
{
	
	//utworz plik o nazwie $name
	puts("otwieram fifo");
	mkfifo(initfifo_name, 0666);
	initfifo_fd = open(initfifo_name, O_RDONLY);

	int tmp_fd = open("sync_fifo", O_WRONLY);
	msg.type = LINK;
	msg.code = -1;
	write(tmp_fd, (char*)&msg, sizeof(msg));	
	close(tmp_fd);
	puts("czekam na pierwszego");	
	while(1)
	{
		
		////sleep(2);
		msg.type = -1;		
		if(read(initfifo_fd, (char*)&msg, sizeof(msg)) == 0 && wrfifo_fd != -1)
		{
			ino_writers_error();
			return -1;
		}
		if(msg.type != -1)
			printf("# INIT # type: %d code:%d\n", msg.type, msg.code);
		
		if(msg.type == LINK)
		{
			if(unmount_state == 1)
			{
				printf("# INIT #: LINK pid:%d - w stanie unmount\n",msg.code);
				u_linkreq();
			}
			else if(wrfifo_fd == -1)
			{
				printf("# INIT #: LINK pid:%d - pierwszy za init\n",msg.code);	
				iadd_new_proc();
			}
			else
			{
				if(secondproc_pid == -1)
					secondproc_pid = msg.code;
				printf("# INIT #: LINK pid:%d\n",msg.code);
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK && unmount_state == 0 && wr_pid != -1)
		{
			printf("# INIT #: UNLINK pid:%d\n",msg.code);
			if(msg.code == -1 && secondproc_pid != -1)
			{
				msg.code = secondproc_pid;
			}
			
						
			write(wrfifo_fd, (char*)&msg, sizeof(msg));
			close(wrfifo_fd);
				
			struct fifo_msg tmpmsg = msg;
			if(tmpmsg.code == -1)
			{		
				printf("nastepnik odlaczanego to init: %d\n", tmpmsg.code);
				wr_pid = -1;
				wrfifo_fd = -1;
				secondproc_pid = -1;
			}		
			else	
			{	
				if(secondproc_pid == tmpmsg.code)
					secondproc_pid = -1;
				printf("nastepnik odlaczanego: %d\n", tmpmsg.code);
				char tmpfifoname[63] = "fifo";
				char tmpstr[20];
				sprintf(tmpstr, "%d", msg.code);
				//const char *tmpstr = itoa(tmpmsg.code);
				wr_fifoname = strcat(tmpfifoname, tmpstr);
				wrfifo_fd = open(tmpfifoname, O_WRONLY);
				wr_pid = tmpmsg.code;
				
			}
			
		}
		else if(msg.type == TOKEN && unmount_state == 0)
		{
			printf("# INIT #: TOKEN \n");
			if(wr_pid != -1 && unmount_state == 0)
			{
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNMOUNT_PREPARE)
		{
			printf("# INIT #: UNMOUNT_PREPARE \n");
			start_unmount();
			if(wr_pid == -1)
			{
				close(initfifo_fd);
				exit(0);
			}
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("# INIT #: UNMOUNT_EXECUTE - konczy sie init \n");
			close(initfifo_fd);
			
			exit(0);
		}
		else if(msg.type == NO_WRITERS)
		{
			no_writers_msg();
		}
	}
	
}

void no_writers_msg()
{
	wr_pid = -1;
	wrfifo_fd = -1;
	secondproc_pid = -1;
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	close(wrfifo_fd);
}

void start_unmount()
{
	unmount_state = 1;
	msg.type = UNMOUNT_EXECUTE;
	msg.code = 0;
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	close(wrfifo_fd);
	unlink(initfifo_name);
}

void ino_writers_error()
{
	struct fifo_msg tmp;
	tmp.type = NO_WRITERS;
	tmp.code = -1;
	write(wrfifo_fd, (char*)&tmp, sizeof(tmp));
	close(wrfifo_fd);
	/*wr_pid = -1;
	wrfifo_fd = -1;
	secondproc_pid = -1;*/
}

void u_linkreq()
{
	char tmpfifoname[80] = "fifo";
	char tmpstr[20];
	sprintf(tmpstr, "%d", msg.code);
	//const char *tmpstr = itoa(msg.code);
	char *temp_fifoname = strcat(tmpfifoname, tmpstr);
	printf("# INIT #: odmawia LINK pid: %d\n", msg.code);
	int tmpfifo_fd = open(temp_fifoname, O_WRONLY);
	msg.type = UNMOUNT_EXECUTE;
	msg.code = 0;
	write(tmpfifo_fd, (char*)&msg, sizeof(msg));
	close(tmpfifo_fd);
}

void iadd_new_proc()
{
	//char tmpfifoname[80] = "fifos/fifo";
	char tmpstr[20];
	sprintf(tmpstr, "%d", msg.code);
	char tmpfifoname[80] = "fifo";
	//const char *tmpstr = itoa(msg.code);
	wr_fifoname = strcat(tmpfifoname, tmpstr);
	wr_pid = msg.code;
	printf("kolejka do pisania inita:\n %s\n",wr_fifoname);
	wrfifo_fd = open(wr_fifoname, O_WRONLY);
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
				
	msg.type=TOKEN;
	msg.code=99999;
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
}
