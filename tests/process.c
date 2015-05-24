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
void no_writers_error();
void add_new_proc();
void unlink();
void clear_fifos();
void initialize();
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
	signal(SIGPIPE, SIG_IGN);	/*sygnał nie będzie generowany przy zapisie do kolejki z której nikt nie czyta*/
	while(1)
	{
		if(fifomutex_lock() != -1)	/*-1 w fifomutex_lock oznacza niepoprawne zakończenie*/
			break;
		puts("WYSTĄPIŁ BŁĄD! ŁĄCZĘ PONOWNIE");
		sleep(1);
	}
	printf("Pracuje proces %d", mypid);
	sleep(3);
	printf("...\n");
	fifomutex_unlock();
	printf("Proces %d odlaczyl sie\n", mypid);
	return 0;
}

int fifomutex_lock()
{	
	mkfifo(myfifo_name, 0666);			/*tworzy własnie fifo i wysyła link do inita*/
	wrfifo_fd = open(initfifo_name, O_WRONLY);
	if(wrfifo_fd == -1)
	{
		printf("Proces pid: %d - kolejka fifo nie istnieje\n",mypid);
		clear_fifos();
		return(-1);
	}
	printf("Proces %d, pisze do: %s\n", mypid, initfifo_name);
	msg.type=LINK;
	msg.code=mypid;
	
	nextproc_pid = -1;
	printf("Proces %d, wysyla LINK\n", mypid);
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	puts(myfifo_name);
	myfifo_fd = open(myfifo_name, O_RDONLY);
	while(1)
	{
		sleep(1);
		if( read(myfifo_fd, (char*)&msg, sizeof(msg)) == 0) /*read==0 oznacza czytanie z kolejki do której nikt nie pisze*/
		{
			no_writers_error();
			return -1;
		}
		printf("Proces %d, type: %d code:%d\n", mypid, msg.type, msg.code);
		if(msg.type == LINK)
		{	
			if(msg.code == mypid)	/*powraca link ode mnie*/
			{
				printf("Proces %d, LINK - powraca\n", mypid);
			}			
			else if(nextproc_pid==-1) /*gdy następny proces to init to dołącz nowy przede mnie*/
			{			
				printf("Proces %d, LINK-nastepny init\n", mypid);
				add_new_proc();
			}
			else
			{
				printf("Proces %d, LINK pid:%d\n", mypid, msg.code);
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK)
		{	
			if(msg.code == nextproc_pid) /*pomimo że badam pid następnego procesu to komunikat ode mnie, w lock() teoretycznie nie może przyjść*/
			{
				printf("Proces %d, UNLINK od siebie, odsyla token\n", mypid);
				unlink();	
				exit(0);
			}
			else
			{
				printf("Proces %d, UNLINK pid:%d\n", mypid, msg.code);			
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == TOKEN)
		{
			return 0;	/*jesli dostałem token to wychodzę*/
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("Proces %d, UNMOUNT_EXECUTE\n", mypid); /* prześlij dalej komunikat o błędzie i odłącz się*/
			write(wrfifo_fd, (char*)&msg, sizeof(msg));
			clear_fifos();
			return -1;
		}
		else if(msg.type == NO_WRITERS)
		{
			no_writers_error(); /*przyszedł błąd krytyczny - muszę się podłączyć ponownie*/
			return -1;
		}
	}
}
int fifomutex_unlock()
{	
	struct fifo_msg unlinkmsg;	/* wysyłam unlink i czekam na jego powrót,w międzyczasie obsługuję inne komunikaty*/
	unlinkmsg.type = UNLINK;
	unlinkmsg.code = nextproc_pid;
	write(wrfifo_fd, (char*)&unlinkmsg, sizeof(unlinkmsg));
	printf("Proces %d, wysyla UNLINK\n", mypid);

	while(1)
	{
		sleep(1);
		if( read(myfifo_fd, (char*)&msg, sizeof(msg)) == 0)
		{
			no_writers_error();
			return -1;
		}
		printf("Proces %d, type: %d code:%d\n", mypid, msg.type, msg.code);
		if(msg.type == LINK)
		{
			
			if(nextproc_pid==-1)
			{
				printf("Proces %d, LINK-nastepny init\n", mypid);
				add_new_proc();
			}
			else
			{
				printf("Proces %d, LINK pid:%d\n", mypid, msg.code);
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK)
		{
			
			if(msg.code == nextproc_pid)
			{
				printf("Proces %d, UNLINK od siebie, odsyla token\n", mypid);	/* gdy wróci mój unlink to oddaję token i zamykam kolejki*/
				unlink();			
				return(0);
			}
			else
			{
				printf("Proces %d, UNLINK pid:%d\n", mypid, msg.code);
				write(wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == TOKEN)
		{
			/*to sie nie moze zdarzyc ale daja na wszelki wypadek*/
			write(wrfifo_fd, (char*)&msg, sizeof(msg));		
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("Proces %d, UNMOUNT_EXECUTE\n", mypid);
			write(wrfifo_fd, (char*)&msg, sizeof(msg));	
			clear_fifos();
			return -1;
		}
		else if(msg.type == NO_WRITERS)
		{
			no_writers_error();
			return -1;
		}
	}
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

void no_writers_error()	/*przesylam dalej komunikat o błędzie i zamykam kolejki*/
{
	struct fifo_msg tmp;
	tmp.type = NO_WRITERS;
	tmp.code = mypid;
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	clear_fifos();
}
void add_new_proc()
{
	int tmpfifo_fd = wrfifo_fd;
	//char fifopath[63] = "fifos/fifo";
	char fifopath[63] = "fifo";
	char *tmpfifoname;
	tmpfifoname = strcat(fifopath, itoa(msg.code));
	printf("teraz pisze do: %s\n", tmpfifoname);
	nextproc_pid = msg.code;
	wrfifo_fd = open(tmpfifoname, O_WRONLY);
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	close(tmpfifo_fd);
}

void unlink()
{
	msg.type=TOKEN;
	msg.code=mypid;
				
	write(wrfifo_fd, (char*)&msg, sizeof(msg));
	clear_fifos();
}

void clear_fifos()
{
	close(wrfifo_fd);
	close(myfifo_fd);
	unlink(myfifo_name);
}


void initialize()
{
	mypid = getpid();
	const char* tmp = itoa(mypid);
	//char fifopath[63] = "fifos/fifo";
	char fifopath[63] = "fifo";
	myfifo_name = strcat(fifopath, tmp);
}


