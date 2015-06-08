#include "../../src/fifo_mutex.h"

#include <stdlib.h>


int main(int argc, char *argv[])
{	
	int ret;
	struct proc_data pd;

	if (argc < 2) {
		printf("Za malo argumentow!\n");
		return -1;
	}
	
	ret=fifomutex_lock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Lock - status: %d\n", getpid(), ret);
		return -1; 
	}

	usleep((unsigned int)strtol(argv[1], NULL, 10));

	ret=fifomutex_unlock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Unlock - status: %d\n", getpid(), ret);
		return -1; 
	}

	return 0;
}
