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
pid_t init_pid;

int initfifo_fd;
char ret[8];

int main(int argc, char *argv[])
{
	/*char *tmp = itoa(45678);
	puts(tmp);*/
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
		struct fifo_msg *msg;
		int tmp_fd=0;
		int wrfifo_fd=-1;
		char *wr_fifoname;
		int wr_pid = -1;
		//utworz plik o nazwie $name
		puts("otwieram fifo");
		mkfifo(initfifo_name, 0666);
		initfifo_fd = open(initfifo_name, O_RDONLY);
		while(1)
		{
			puts("czekam na pierwszego");
			read(initfifo_fd, msg, sizeof(msg));
			puts("chujnia");
			if(msg->type == LINK)
			{puts("chujnia1");
				if(wrfifo_fd == -1)
				{
					printf("# INIT #: LINK pid:%d - pierwszy za init",msg->code);
					char tmpfifoname[60] = "/home/szymon/Dokumenty/SFS UXP1A/UXP1A-fs-fifo/tests/fifo";
					const char *tmpstr = itoa(msg->code);
					wr_fifoname = strcat(tmpfifoname, tmpstr);
					wr_pid = msg->code;
					wrfifo_fd = open(wr_fifoname, O_WRONLY);
					write(wrfifo_fd, msg, sizeof(msg));
					struct fifo_msg tmpmsg;
					tmpmsg.type=TOKEN;
					tmpmsg.code=0;
					msg = &tmpmsg;
					write(wrfifo_fd, msg, sizeof(tmpmsg));
				}
				else
				{
					printf("# INIT #: LINK pid:%d",msg->code);
					write(wrfifo_fd, msg, sizeof(msg));
				}
			}
			else if(msg->type == UNLINK)
			{puts("chujnia2");
				if(wr_pid == msg->code)
				{
					printf("# INIT #: UNLINK pid:%d - nastepny po init",msg->code);
					 				
					write(wrfifo_fd, msg, sizeof(msg));
					close(wrfifo_fd);
					if(msg->code_2 == -1)
					{
						wrfifo_fd = open(initfifo_name, O_RDONLY);
						wr_pid = -1;
					}		
					else	
					{
						char tmpfifoname[60] = "/home/szymon/Dokumenty/SFS UXP1A/UXP1A-fs-fifo/tests/fifo";
						const char *tmpstr = itoa(msg->code_2);
						wr_fifoname = strcat(tmpfifoname, tmpstr);
						wrfifo_fd = open(tmpfifoname, O_RDONLY);
						wr_pid = msg->code_2;
					}
				}
				else
				{
					write(wrfifo_fd, msg, sizeof(msg));
				}
			}
			else if(msg->type == TOKEN)
			{puts("chujnia3");
				printf("# INIT #: TOKEN ");
				if(wr_pid != -1)
				{
					write(wrfifo_fd, msg, sizeof(msg));
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
	return ret;
}







