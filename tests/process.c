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
void frommsg(struct fifo_msg *msg, char *tmp);
void tomsg(struct fifo_msg *msg, char* buf);

int wrfifo_fd, myfifo_fd;
char *myfifo_name;
char ret[8];
char tmpstr[11];
int pom_count=5;
int nextproc_pid = -1;
int mypid;
struct fifo_msg msg;

int main(int argc, char *argv[])
{
	mypid = getpid();
	const char* tmp = itoa(mypid);
	//char fifopath[63] = "fifos/fifo";
	char fifopath[63] = "fifo";
	myfifo_name = strcat(fifopath, tmp);

	puts("dupa");
	fifomutex_lock();
	return 0;
}

int fifomutex_lock()
{
	
	mkfifo(myfifo_name, 0666);
	
	wrfifo_fd = open(initfifo_name, O_WRONLY);
	printf("Proces %d, pisze do: %s\n", mypid, initfifo_name);
	msg.type=LINK;
	msg.code=mypid;
	msg.code_2=-1;
	nextproc_pid = -1;
	printf("Proces %d, wysyla LINK\n", mypid);
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	puts(myfifo_name);
	myfifo_fd = open(myfifo_name, O_RDONLY);
	while(1)
	{
		sleep(1);
		if( read(myfifo_fd, (char*)&msg, sizeof(msg)) == 0)
			continue;
		printf("Proces %d, type: %d code:%d code_2: %d\n", mypid, msg.type, msg.code, msg.code_2);
		if(msg.type == LINK)
		{
			
			if(msg.code == mypid)
			{
				printf("Proces %d, LINK - powraca, poprzednik: %d\n", mypid, msg.code_2);
				//nextproc_pid = msg.code_2;
			}			
			else if(nextproc_pid==-1)
			{
				printf("Proces %d, LINK-nastepny init\n", mypid);
				int tmpfifo_fd = wrfifo_fd;
				//char fifopath[63] = "fifos/fifo";
				char fifopath[63] = "fifo";
				char *tmpfifoname;
				tmpfifoname = strcat(fifopath, itoa(msg.code));
				printf("teraz pisze do: %s\n", tmpfifoname);
				nextproc_pid = msg.code;
				wrfifo_fd = open(tmpfifoname, O_WRONLY);
				msg.code_2 = mypid;
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(tmpfifo_fd);
			}
			else
			{
				printf("Proces %d, LINK pid:%d\n", mypid, msg.code);
				msg.code_2 = mypid;
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK)
		{
			
			if(msg.code == mypid)
			{
				printf("Proces %d, UNLINK od siebie, odsyla token\n", mypid);
				msg.type=TOKEN;
				msg.code=mypid;
				msg.code_2=99999;
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(wrfifo_fd);
				close(myfifo_fd);
				unlink(myfifo_name);
				
				exit(0);
			}
			else if(msg.code == nextproc_pid)
			{
				printf("Proces %d, UNLINK-nastepny proces sie odlacza next %d\n", mypid, nextproc_pid);
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(wrfifo_fd);
				if(msg.code_2 == -1)
				{
					wrfifo_fd = open(initfifo_name, O_RDONLY);
					nextproc_pid = -1;
				}		
				else	
				{
					char *tmpfifoname;
					//char fifopath[63] = "fifos/fifo";
					char fifopath[63] = "fifo";
					tmpfifoname = strcat(fifopath, itoa(msg.code_2));
					wrfifo_fd = open(tmpfifoname, O_WRONLY);
			
				}
			}
			else
			{
				printf("Proces %d, UNLINK pid:%d\n", mypid, msg.code);
			
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == TOKEN)
		{
			if(pom_count<=0)
			{
				fifomutex_unlock();
		
			}
			else
			{
				printf("Pracuje proces %d, %d. obieg\n", mypid, 6-pom_count);
			sleep(1);
			/*
				pracuje...
			*/
			pom_count--;
			msg.code = mypid;

			write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}		
		}
	}
}
int fifomutex_unlock()
{	
	struct fifo_msg unlinkmsg;
	char tmpstr2[11];
	unlinkmsg.type = UNLINK;
	unlinkmsg.code = mypid;
	unlinkmsg.code_2 = nextproc_pid;
	write(wrfifo_fd, (char*)&unlinkmsg, sizeof(unlinkmsg));
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



