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
#include "const.h"

char *itoa(int number);
int fifomutex_lock();
int fifomutex_unlock();

int wrfifo_fd, myfifo_fd;
char *myfifo_name;
char ret[8];
int pom_count=5;
int nextproc_pid = -1;
int mypid;
struct fifo_msg *msg;
char fifopath[60] = "/home/szymon/Dokumenty/SFS UXP1A/UXP1A-fs-fifo/tests/fifo";
int main(int argc, char *argv[])
{
	mypid = getpid();
	const char* tmp = itoa(mypid);
	myfifo_name = strcat(fifopath, tmp);

	puts("dupa");
	fifomutex_lock();
	return 0;
}

int fifomutex_lock()
{
	
	mkfifo(myfifo_name, 0666);
	
	wrfifo_fd = open(initfifo_name, O_WRONLY);
	struct fifo_msg tmpmsg;
	tmpmsg.type=LINK;
	tmpmsg.code=mypid;
	msg = &tmpmsg;
	printf("Proces %d, wysyla LINK\n", mypid);
	write(wrfifo_fd, msg, sizeof(msg));
	//int undone=1;
	myfifo_fd = open(myfifo_name, O_WRONLY);
	while(1)
	{
		read(myfifo_fd, msg, sizeof(msg));
		if(msg->type == LINK)
		{
			
			if(nextproc_pid==-1)
			{
				printf("Proces %d, LINK-nastepny init\n", mypid);
				int tmpfifo_fd;
				char *tmpfifoname;
				tmpfifoname = strcat(fifopath, itoa(msg->code));
				nextproc_pid = msg->code;
				wrfifo_fd = open(tmpfifoname, O_WRONLY);
				write(wrfifo_fd, msg, sizeof(msg));
				close(tmpfifo_fd);
			}
			else
			{
				printf("Proces %d, LINK pid:%d\n", mypid, msg->code);
				write(wrfifo_fd, msg, sizeof(msg));
			}
		}
		else if(msg->type == UNLINK)
		{
			
			if(msg->code == mypid)
			{
				printf("Proces %d, UNLINK od siebie\n", mypid);
				close(wrfifo_fd);
				unlink(myfifo_name);
				close(myfifo_fd);
				return 0;
			}
			else if(msg->code == nextproc_pid)
			{
				printf("Proces %d, UNLINK-nastepny proces sie odlacza\n", mypid);
				write(wrfifo_fd, msg, sizeof(msg));
				close(wrfifo_fd);
				if(msg->code_2 == -1)
				{
					wrfifo_fd = open(initfifo_name, O_RDONLY);
					nextproc_pid = -1;
				}		
				else	
				{
					char *tmpfifoname;
					tmpfifoname = strcat(fifopath, itoa(msg->code_2));
					wrfifo_fd = open(tmpfifoname, O_RDONLY);
					nextproc_pid = msg->code_2;
				}
			}
			else
			{
				printf("Proces %d, UNLINK pid:%d\n", mypid, msg->code);
				write(wrfifo_fd, msg, sizeof(msg));
			}
		}
		else if(msg->type == TOKEN)
		{
			printf("Pracuje proces %d, %d. obieg\n", mypid, 6-pom_count);
			/*
				pracuje...
			*/
			pom_count--;

			write(wrfifo_fd, msg, sizeof(msg));	

			if(pom_count<=0)
			{
				fifomutex_unlock();
				//undone=0;
			}
		}
	}
}
int fifomutex_unlock()
{	
	struct fifo_msg unlinkmsg;
	unlinkmsg.type = UNLINK;
	unlinkmsg.code = mypid;
	msg = &unlinkmsg;
	write(wrfifo_fd, msg, sizeof(unlinkmsg));
	printf("Proces %d, wysyla UNLINK\n", mypid);
}
/*sam to cholerstwo napisałem, ale dziala jak cos*/
char *itoa(int number)
{	
	int tens=10000, cnt=0, i, j;
	char str[8];
	
	str[cnt++] = number%10;
	while(tens>0)
	{	
		tens = number/10;
		str[cnt] = number%10;
		number = tens;
		++cnt;
	}
	for(i=cnt-1, j=0; i>0; i--, j++)
	{
		ret[j] = str[i]+48;
	}
	ret[j] = '\0';
	return ret;
}



