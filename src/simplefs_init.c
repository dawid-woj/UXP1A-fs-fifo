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

#define N 512

char *initfifo_name = "initfifo";

int fifomutex_init();

int pid_buf[N];	    // Bufor cykliczny procesow w token ringu
int first_free = 0; // pierwsze wolne miejsce
int last_taken = 0; // ostatnie zajete miejsce

int initfifo_fd;
	
int unmount_state=0;		
	
struct fifo_msg msg;
int wrfifo_fd=-1;
char *wr_fifoname;
int wr_pid = -1;
int ret=0;

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
	puts("# INIT # otwieram fifo");
	if(mkfifo(initfifo_name, 0666) == -1)
	{
		printf("# INIT # nie można utworzyć fifo");
		return -1;
	}
	initfifo_fd = open(initfifo_name, O_RDWR);
	if(initfifo_fd == -1)
	{
		printf("# INIT # nie można otworzyc fifo");
		unlink(initfifo_name);
		return -1;
	}
	puts("# INIT # otwieram fifo synchronizujace");
	int tmp_fd = open("sync_fifo", O_WRONLY);
	if(tmp_fd == -1)
	{
		printf("# INIT # nie można otworzyc fifo synchronizujacego");
		unlink(initfifo_name);
		close(initfifo_fd);
		return -1;
	}

	msg.type = LINK;
	msg.code = -1;
	write(tmp_fd, (char*)&msg, sizeof(msg));	/*pomijam obsluge bledow, nie ma sensu na razie*/
	close(tmp_fd);

	puts("# INIT # czekam na pierwszego");	
	while(1)
	{		
		if(read(initfifo_fd, (char*)&msg, sizeof(msg)) == 0) /* To sie chyba nie powinno zdarzyc ??? */
		{
			puts("# INIT # odebrano pusty komunikat");
			ino_writers_error();
			return -1;
		}
		if(msg.type == LINK)
		{
			pid_buf[first_free] = msg.code;
			first_free = (first_free + 1) % N;
			
			if(unmount_state == 1)
			{
				printf("# INIT # LINK od pid:%d - w stanie unmount - odmowa linkowania\n",msg.code);
				u_linkreq();
			}
			else if(wrfifo_fd == -1)
			{
				printf("# INIT # LINK od pid:%d - dodaje nowy proces za initem i przekazuje TOKEN\n",msg.code);
				iadd_new_proc();
			}
			else
			{
				printf("# INIT # LINK od pid:%d - przekazuje dalej\n",msg.code);
				ret = write(wrfifo_fd, (char*)&msg, sizeof(msg));
				if(ret == -1 && errno == EPIPE)
					return -2;					
			}
		}
		else if(msg.type == UNLINK)
		{
		  
			
			if(unmount_state)
			{
			  printf("# INIT # UNLINK od %d ale jestesmy w stanie umount -> ignorowanie\n", msg.upid);
			  continue;
			}

			last_taken = (last_taken + 1) % N;
			int tmp_fd = wrfifo_fd;
			
			if(last_taken == first_free) // Odlacza sie jedyny proces w pierscieniu
			{
				printf("# INIT # UNLINK %d i nie ma innych\n", msg.upid);
				wr_pid = -1;
				wrfifo_fd = -1;
			}
			else // Sa inne procesy, trzeba przepiac kolejki
			{
				printf("# INIT # UNLINK %d - odlacza sie proces za - przepiecie kolejek\n", msg.upid);
				char tmpstr[20];
				sprintf(tmpstr, "fifo%d", pid_buf[last_taken]);
				wr_fifoname = tmpstr;
				wrfifo_fd = open(wr_fifoname, O_WRONLY);
				wr_pid = pid_buf[last_taken];
			}
			ret = write(wrfifo_fd, (char*)&msg, sizeof(msg));
			if(ret == -1 && errno == EPIPE)
				return -2;
			close(tmp_fd);
		}
		else if(msg.type == TOKEN)
		{
			//if( (first_free-last_taken) <= 2 && msg.code != pid_buf[(first_free - (first_free-last_taken) + N) % N])
			if( (first_free-last_taken) == 1 && msg.code != pid_buf[(first_free - 1 + N) % N])
			{
				printf("# INIT # TOKEN od %d wrocil do inita - UDAREMNIAM BLAD\n", msg.upid);
			}
			else if(wr_pid != -1 && unmount_state == 0)
			{
				printf("# INIT # TOKEN od %d wrocil do inita - przekazujemy dalej\n", msg.upid);
				ret = write(wrfifo_fd, (char*)&msg, sizeof(msg));
				if(ret == -1 && errno == EPIPE)
					return -2;
			}
		}
		else if(msg.type == UNMOUNT_PREPARE)
		{
			printf("# INIT # UNMOUNT_PREPARE \n");
			start_unmount();
			if(wr_pid == -1)
			{
				close(initfifo_fd);
				exit(0);
			}
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("# INIT # UNMOUNT_EXECUTE - konczy sie init \n");
			close(initfifo_fd);
			
			exit(0);
		}
		else if(msg.type == NO_WRITERS)
		{
			printf("# INIT # NO_WRITERS\n");
			no_writers_msg();
			last_taken = first_free = 0;
		}
		else
		{
		  printf("# INIT # Nieprzewidziana sytuacja - type:%d, code: %d, upid: %d!!!\n", msg.type, msg.code, msg.upid);
		}
	}
	
}

void no_writers_msg()
{	
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	close(wrfifo_fd);
	wr_pid = -1;
	wrfifo_fd = -1;
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
}

void u_linkreq()
{
	char tmpstr[20];
	sprintf(tmpstr, "fifo%d", msg.code);
	char *temp_fifoname = tmpstr;
	int tmpfifo_fd = open(temp_fifoname, O_WRONLY);
	msg.type = UNMOUNT_EXECUTE;
	msg.code = 0;
	write(tmpfifo_fd, (char*)&msg, sizeof(msg));
	close(tmpfifo_fd);
}

void iadd_new_proc()
{
	char tmpstr[20];
	sprintf(tmpstr, "fifo%d", msg.code);
	wr_fifoname = tmpstr;
	wr_pid = msg.code;
	wrfifo_fd = open(wr_fifoname, O_WRONLY);
	write(wrfifo_fd, (char*)&msg, sizeof(msg));		
	msg.type=TOKEN;
	msg.code=-1;
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
}
