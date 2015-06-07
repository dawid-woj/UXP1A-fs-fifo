#include "../../src/fifo_mutex.h"

int main(int argc, char *argv[])
{	
	int cnt = 0;
	int ret = -10000;
	struct proc_data pd;
	//signal(SIGPIPE, SIG_IGN);
	while( (ret=fifomutex_lock(&pd)) <= -1)
	{
		if(ret == -1)
		{
			cnt++;
			if(cnt>3)
				break;	
		}	
		else if(ret == -2)
			{}
	}
	sleep(3);
	if(argc >= 2 && (strcmp(argv[1], "-u") == 0))
	{
		fifomutex_unlock(&pd);
	}
	else if(cnt<=3)
		fifomutex_unlock(&pd);


	cnt = 0;
	ret = -10000;
	while( (ret=fifomutex_lock(&pd)) <= -1)
	{
		if(ret == -1)
		{
			cnt++;
			if(cnt>3)
				break;	
		}	
		else if(ret == -2)
			{}
	}
	sleep(3);
	if(argc >= 2 && (strcmp(argv[1], "-u") == 0))
	{
		fifomutex_unlock(&pd);
	}
	else if(cnt<=3)
		fifomutex_unlock(&pd);
	return 0;
}
