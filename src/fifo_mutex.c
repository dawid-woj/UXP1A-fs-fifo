#include "fifo_mutex.h"

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/
char *initfifo_name = "initfifo";

pid_t init_pid = -1;
/***************************************************************************
 * API modulu
 **************************************************************************/

/***************************************************************************
 * FIFOMUTEX_LOCK
 * daje dostep do sekcji krytycznej
 * Parametry:
 * data - struktura z danymi potrzebnymi do obsllugi dzialania pierscienia
 *  z fifo, -- glownym jej celem jest zapamietanie danych pomiedzy wywolaniem
 *  lock() i unlock()
 * Zwraca:
 * 0 - wszystko sie udalo, teraz proces ma wylaczny dostep do systemu
 * -1 - proces zostal zakonczony wywolaniem f-cji fifomutex_unmount()
 * -2 - proces zostal zakonczony z powodu bledu odczytu
 *
 * Wazne: kazdy proces wywolujacy ta funckje musi zablokowac sygnal SIG_PIPE:
 *  signal(SIGPIPE, SIG_IGN);
 **************************************************************************/

int fifomutex_lock(struct proc_data *data)
{	
	signal(SIGPIPE, SIG_IGN);
	struct fifo_msg msg;
	int mypid = getpid();
	
	initialize(data, mypid);
	mkfifo(data->myfifo_name, 0666);			/*tworzy własnie fifo i wysyła link do inita*/
	data->wrfifo_fd = open(initfifo_name, O_WRONLY);
	if(data->wrfifo_fd == -1)
	{
		printf("#%d [L] kolejka fifoinit nie istnieje\n",mypid);
		clear_fifos(data);
		return(-1);
	}
	printf("#%d [L] pisze do: %s\n", mypid, initfifo_name);
	msg.type = LINK;
	msg.code = mypid;
	msg.upid = mypid;
	

	data->nextproc_pid = -1;
	printf("#%d [L] wysyla LINK do inita\n", mypid);
	write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
	puts(data->myfifo_name);
	data->myfifo_fd = open(data->myfifo_name, O_RDONLY);
	while(1)
	{
		if( read(data->myfifo_fd, (char*)&msg, sizeof(msg)) == 0) /*read==0 oznacza czytanie z kolejki do której nikt nie pisze*/
		{
			printf("#%d [L] brak czytelnikow!\n",mypid);
			no_writers_error(data, mypid);
			return -2;
		}
		if(msg.type == LINK)
		{	
			if(msg.code == mypid)	/*powraca link ode mnie*/
			{
				printf("#%d [L] LINK - powraca - podlaczony do token ring\n", mypid);
			}			
			else if(data->nextproc_pid==-1) /*gdy następny proces to init to dołącz nowy przede mnie*/
			{			
				printf("#%d [L] LINK - linkuje przed soba %d\n", mypid, msg.code);
				add_new_proc(data, &msg);
			}
			else
			{
				printf("#%d [L] LINK od pid:%d - przekazuje dalej\n", mypid, msg.code);
				write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK)
		{	
			if(msg.upid == mypid) /*pomimo że badam pid następnego procesu to komunikat ode mnie, w lock() teoretycznie nie może przyjść*/
			{
				printf("#%d [L] UNLINK powraca - odsyla token - CANNOT BE!!!\n", mypid);
				fifo_unlink(mypid, data, &msg);	
				exit(0);
			}
			else
			{
				printf("#%d [L] UNLINK od pid:%d za ktorym jest:%d\n", mypid, msg.upid, msg.code);			
				write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == TOKEN)
		{
			printf("#%d [L] TOKEN otrzymany\n", mypid);
			return 0;	/*jesli dostałem token to wychodzę*/
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("#%d [L] UNMOUNT_EXECUTE\n", mypid); /* prześlij dalej komunikat o błędzie i odłącz się*/
			write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
			clear_fifos(data);
			return -1;
		}
		else if(msg.type == NO_WRITERS)
		{
			printf("#%d [L] NO_WRITERS od %d\n", mypid, msg.upid);
			no_writers_error(data, mypid); /*przyszedł błąd krytyczny - muszę się podłączyć ponownie*/
			return -2;
		}
		else
		{
		  printf("#%d [L] Nieprzewidziana sytuacja - type:%d, code: %d, upid: %d!!!\n", mypid, msg.type, msg.code, msg.upid);
		}
	}
}


/***************************************************************************
 * FIFOMUTEX_UNLOCK
 * zwalnia dostep do sekcji krytycznej
 * Parametry:
 * data - struktura z danymi potrzebnymi do obslugi dzialania pierscienia
 *  z fifo; Musi to byc dokladnie ta sama struktura z ktora wywolalismy
 *  lock(), inaczej operacja sie nie powiedzie
 * Zwraca:
 * 0 - wszystko sie udalo, teraz proces ma wylaczny dostep do systemu
 * -1 - proces zostal zakonczony wywolaniem f-cji fifomutex_unmount()
 * -2 - proces zostal zakonczony z powodu bledu odczytu
 *
 * Wazne: kazdy proces wywolujacy ta funckje musi zablokowac sygnal SIG_PIPE:
 *  signal(SIGPIPE, SIG_IGN);
 **************************************************************************/
int fifomutex_unlock(struct proc_data *data)
{	
	
	int mypid = getpid();
	if(data->wrfifo_fd == -1 && data->myfifo_fd == -1)
	{
		printf("#%d [U] złe dane\n",mypid);
		close(data->wrfifo_fd);
		close(data->myfifo_fd);
		return(-1);
	}
	
	struct fifo_msg msg;

	struct fifo_msg unlinkmsg;	/* wysyłam unlink i czekam na jego powrót,w międzyczasie obsługuję inne komunikaty*/
	unlinkmsg.type = UNLINK;
	unlinkmsg.code = data->nextproc_pid;
	unlinkmsg.upid = mypid;
	write(data->wrfifo_fd, (char*)&unlinkmsg, sizeof(unlinkmsg));
	if(data->wrfifo_fd == -1)
	{
		printf("#%d [U] kolejka fifo nie istnieje\n",mypid);
		clear_fifos(data);
		free(data->myfifo_name);
		return(-1);
	}
	printf("#%d [U] wysyla UNLINK\n", mypid);

	while(1)
	{
		if( read(data->myfifo_fd, (char*)&msg, sizeof(msg)) == 0) /*read==0 oznacza czytanie z kolejki do której nikt nie pisze*/
		{
			printf("#%d [U] brak czytelnikow!\n",mypid);
			no_writers_error(data, mypid);
			free(data->myfifo_name);
			return -2;
		}	
		if(msg.type == LINK)
		{	
			if(data->nextproc_pid==-1)
			{
				printf("#%d [U] LINK %d - nastepny init - linkuje przed soba\n", mypid, msg.code);
				add_new_proc(data, &msg);
			}
			else
			{
				printf("#%d [U] LINK pid:%d - przekazuje dalej\n", mypid, msg.code);
				write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == UNLINK)
		{
			
			if(msg.upid == mypid)
			{
				printf("#%d [U] UNLINK od siebie - odsyla token\n", mypid);	/* gdy wróci mój unlink to oddaję token i zamykam kolejki*/
				fifo_unlink(mypid, data, &msg);	
				free(data->myfifo_name);		
				return(0);
			}
			else
			{
				printf("#%d [U] UNLINK od innego procesu pid:%d - ROZMNOZENIE TOKENA!!! - nie przekazuje dalej\n", mypid, msg.code);
				//write(data->wrfifo_fd, (char*)&msg, sizeof(msg));
			}
		}
		else if(msg.type == TOKEN)
		{
			/*to sie nie moze zdarzyc ale daja na wszelki wypadek*/
			printf("#%d [U] TOKEN od %d - ROZMNOZENIE TOKENA!!! -- nie przekazuje dalej\n", mypid, msg.code);
			//write(data->wrfifo_fd, (char*)&msg, sizeof(msg));		
		}
		else if(msg.type == UNMOUNT_EXECUTE)
		{
			printf("#%d [U] UNMOUNT_EXECUTE\n", mypid);
			write(data->wrfifo_fd, (char*)&msg, sizeof(msg));	
			clear_fifos(data);
			free(data->myfifo_name);
			return -1;
		}
		else if(msg.type == NO_WRITERS)
		{
			printf("#%d [U] NO_WRITERS od %d\n", mypid, msg.upid);
			no_writers_error(data, mypid);
			free(data->myfifo_name);
			return -2;
		}
		else
		{
		  printf("#%d [U] Nieprzewidziana sytuacja - type:%d, code: %d, upid: %d!!!\n", mypid, msg.type, msg.code, msg.upid);
		}
	}
}


void no_writers_error(struct proc_data *data, int mypid)	/*przesylam dalej komunikat o błędzie i zamykam kolejki*/
{
	
	struct fifo_msg tmp;
	tmp.type = NO_WRITERS;
	tmp.code = mypid;
	tmp.upid = mypid;
	write(data->wrfifo_fd, (char*)&tmp, sizeof(tmp));
	clear_fifos(data);
}
void add_new_proc(struct proc_data *data, struct fifo_msg *msg)
{
	int tmpfifo_fd = data->wrfifo_fd;
	char *tmpfifoname;
	char tmpstr[20];
	sprintf(tmpstr, "fifo%d", msg->code);
	printf("sprintf: %s\n",tmpstr);
	tmpfifoname = tmpstr;
	//printf("teraz pisze do: %s\n", tmpfifoname);
	data->nextproc_pid = msg->code;
	data->wrfifo_fd = open(tmpfifoname, O_WRONLY);
	write(data->wrfifo_fd, (char*)msg, sizeof(struct fifo_msg));
	close(tmpfifo_fd);
}

void fifo_unlink(int mypid, struct proc_data *data, struct fifo_msg *msg)
{
	msg->type=TOKEN;
	msg->code=mypid;
	msg->upid=mypid;
				
	write(data->wrfifo_fd, (char*)msg, sizeof(struct fifo_msg));
	clear_fifos(data);
}

void clear_fifos(struct proc_data *data)
{
	close(data->wrfifo_fd);
	close(data->myfifo_fd);
	unlink(data->myfifo_name);
	
	data->wrfifo_fd = -1;
	data->myfifo_fd = -1;
}


void initialize(struct proc_data *data, int mypid)
{
	
	char tmpstr[20];
	sprintf(tmpstr, "fifo%d", mypid);
	//printf("sprintf: %s\n",tmpstr);
	char *fifopath = malloc(20*sizeof(char));
	memcpy(fifopath, tmpstr, 20);
	data->myfifo_name = fifopath;
	
}


/***************************************************************************
 * FIFOMUTEX_STARTINIT
 * tworzy proces init
 * Parametry:
 * 
 * Zwraca:
 * 0 - wszystko sie udalo
 * -1 - proces nie zostal utworzony
 **************************************************************************/
int fifomutex_startinit()
{
	puts("sprawdzam czy istnieje proces init");
	int tmp;
	if((tmp = open(initfifo_name, O_RDWR)) != -1)
	{
		puts("istnieje");
		close(tmp);
		return(-1);
	}
	puts("nie isnieje - tworze init");
	close(tmp);

	init_pid = fork();
	int ret = 0;			/*exec przy udanej operacji nie zwraca nic
					, przy bledzie -1*/
	if(init_pid == 0)
	{
		ret = execl("./simplefs_init", "simplefs_init", (char*)0);
		return ret;
	}
	puts("otwieram kolejkę synchronizującą");
	struct fifo_msg msg;
	mkfifo("sync_fifo", 0666);			/*tworzy własnie fifo i wysyła link do inita*/
	int tmp_fd = open("sync_fifo", O_RDONLY);
	read(tmp_fd, (char*)&msg, sizeof(msg));
	close(tmp_fd);
	unlink("sync_fifo");
	puts("zamykam kolejkę synchronizującą");

	return ret;
}

/***************************************************************************
 * FIFOMUTEX_UMOUNT
 * powoduje usuniecie kolejek pierscienia, zakonczenie procesu init
 * Wazne: czeka az init sie zakonczy
 * Parametry:
 * 
 * Zwraca:
 * 0 - wszystko sie udalo
 * tu musze jeszcze dopracowac
 **************************************************************************/
int fifomutex_umount()
{

	struct fifo_msg msg;
	int tmpfifo_fd = -1;
	if((tmpfifo_fd = open(initfifo_name, O_WRONLY)) == -1)
	{
		puts("Fifomutex unmount: brak kolejki init");
		return -1;	
	}
	msg.type = UNMOUNT_PREPARE;
	msg.code = 0;
	write(tmpfifo_fd, (char*)&msg, sizeof(msg));
	close(tmpfifo_fd);
	int status;
	waitpid(init_pid, &status, 0);
	return 0;
}
