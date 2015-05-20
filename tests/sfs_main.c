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
void tomsg(struct fifo_msg *msg, char* buf);
void frommsg(struct fifo_msg *msg, char *buf);
int simplefs_mount(char* name);
int simplefs_umount();
pid_t init_pid;
int secondproc_pid=-1;
int initfifo_fd;
char ret[8];
char tmpstr[11];
int main(int argc, char *argv[])
{
	simplefs_mount("sfsfile");
	puts("tworze fifo, nacisnij przycisk aby usunac");
	getchar();
	simplefs_umount();
}



int simplefs_mount(char* name)
{
	
	/*init_pid = fork();
	if(init_pid == 0)
	{	*/
		struct fifo_msg msg;
		int tmp_fd=0;
		int wrfifo_fd=-1;
		char *wr_fifoname;
		int wr_pid = -1;
		//utworz plik o nazwie $name
		puts("otwieram fifo");
		mkfifo(initfifo_name, 0666);
		initfifo_fd = open(initfifo_name, O_RDONLY);
		puts("czekam na pierwszego");
		while(1)
		{
			
			sleep(2);
			if( read(initfifo_fd, (char*)&msg, sizeof(msg)) == 0)
				continue;
			printf("# INIT # type: %d code:%d\n", msg.type, msg.code);
			
			if(msg.type == LINK)
			{
				
				if(wrfifo_fd == -1)
				{
					printf("# INIT #: LINK pid:%d - pierwszy za init\n",msg.code);
					//char tmpfifoname[80] = "fifos/fifo";
					char tmpfifoname[80] = "fifo";
					const char *tmpstr = itoa(msg.code);
					wr_fifoname = strcat(tmpfifoname, tmpstr);
					wr_pid = msg.code;
					printf("kolejka do pisania inita:\n %s\n",wr_fifoname);
					wrfifo_fd = open(wr_fifoname, O_WRONLY);
					write(wrfifo_fd, (char*)&msg, sizeof(msg));
					
					msg.type=TOKEN;
					msg.code=99999;
					write(wrfifo_fd, (char*)&msg, sizeof(msg));
				}
				else
				{
					if(secondproc_pid == -1)
						secondproc_pid = msg.code;
					printf("# INIT #: LINK pid:%d\n",msg.code);
					write(wrfifo_fd, (char*)&msg, sizeof(msg));
				}
			}
			else if(msg.type == UNLINK)
			{
				if(msg.code == -1 && secondproc_pid != -1)
				{
					msg.code = secondproc_pid;
				}
				printf("# INIT #: UNLINK pid:%d 1-\n",msg.code);
							
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(wrfifo_fd);
					
				printf("# INIT #: UNLINK pid:%d 2-\n",msg.code);
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
					const char *tmpstr = itoa(tmpmsg.code);
					wr_fifoname = strcat(tmpfifoname, tmpstr);
					wrfifo_fd = open(tmpfifoname, O_WRONLY);
					wr_pid = tmpmsg.code;
					
				}
				
			}
			else if(msg.type == TOKEN)
			{
				printf("# INIT #: TOKEN \n");
				if(wr_pid != -1)
				{
					write(wrfifo_fd, (char*)&msg, sizeof(msg));
				}
			}
		}
		puts("wyszedlem z while");
		
	//}
}
int simplefs_umount()
{
	unlink(initfifo_name);
	close(initfifo_fd);
	kill(init_pid, SIGKILL);
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
	puts(ret);
	return ret;
}



