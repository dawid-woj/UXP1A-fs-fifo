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

#include "const.h"

char *itoa(int number);
void tomsg(struct fifo_msg *msg, char* buf);
void frommsg(struct fifo_msg *msg, char *buf);
int simplefs_mount(char* name);
int simplefs_umount();
pid_t init_pid;
char ret[8];



int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);
	simplefs_mount("sfsfile");
	puts("tworze fifo");
	//getchar();
	/*tu jest słabo z testowaniem bo proces dziecko wypisuje na ekran i getchar nie reaguje*/
	//sleep(13);
	puts("usuwam");
	//simplefs_umount();
	return 0;
}



int simplefs_mount(char* name)
{

	init_pid = fork();
	if(init_pid == 0)
	{
		int secondproc_pid=-1;
		int initfifo_fd;
		
		//char tmpstr[11];
		int unmount_state=0;		
			
		struct fifo_msg msg;
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
			if( read(initfifo_fd, (char*)&msg, sizeof(msg)) == 0 && wrfifo_fd != -1)
			{
				struct fifo_msg tmp;
				tmp.type = NO_WRITERS;
				tmp.code = -1;
				write(wrfifo_fd, (char*)&tmp, sizeof(tmp));
				close(wrfifo_fd);
				//close(initfifo_fd);
				//unlink(initfifo_name);
				return -1;
			}
			printf("# INIT # type: %d code:%d\n", msg.type, msg.code);
			
			if(msg.type == LINK)
			{
				if(unmount_state == 1)
				{
					printf("# INIT #: LINK pid:%d - w stanie unmount\n",msg.code);
					char tmpfifoname[80] = "fifo";
					const char *tmpstr = itoa(msg.code);
					char *temp_fifoname = strcat(tmpfifoname, tmpstr);
					printf("# INIT #: odmawia LINK pid: %d\n", msg.code);
					int tmpfifo_fd = open(temp_fifoname, O_WRONLY);
					msg.type = UNMOUNT_EXECUTE;
					msg.code = 0;
					write(tmpfifo_fd, (char*)&msg, sizeof(msg));
					close(tmpfifo_fd);
				}
				else if(wrfifo_fd == -1)
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
			else if(msg.type == UNLINK && unmount_state == 0)
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
				unmount_state = 1;
				msg.type = UNMOUNT_EXECUTE;
				msg.code = 0;
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(wrfifo_fd);
				unlink(initfifo_name);
			}
			else if(msg.type == UNMOUNT_EXECUTE)
			{
				printf("# INIT #: UNMOUNT_EXECUTE - konczy sie init \n");
				close(initfifo_fd);
				
				exit(0);
			}
			else if(msg.type == NO_WRITERS)
			{
				wr_pid = -1;
				wrfifo_fd = -1;
				secondproc_pid = -1;
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
				close(wrfifo_fd);
			}
		}
		
	}
	return 0;
}
int simplefs_umount()
{
	/*unlink(initfifo_name);
	close(initfifo_fd);
	kill(init_pid, SIGKILL);*/
	struct fifo_msg msg;
	int tmpfifo_fd = open(initfifo_name, O_WRONLY);
	msg.type = UNMOUNT_PREPARE;
	msg.code = 0;
	write(tmpfifo_fd, (char*)&msg, sizeof(msg));
	close(tmpfifo_fd);
	int status;
	waitpid(init_pid, &status, 0);//WUNTRACED);
	return 0;
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



