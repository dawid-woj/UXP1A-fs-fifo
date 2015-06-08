#include "../../src/fifo_mutex.h"


int main(int argc, char *argv[])
{	
	int ret;
	struct proc_data pd;

	printf("proc1 melduje sie\n");
	
	// OPERACJA NR 1
	ret=fifomutex_lock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Lock nr 1 - status: %d\n", getpid(), ret);
		return -1; 
	}

	sleep(1); // 1 s

	ret=fifomutex_unlock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Unlock nr 1 - status: %d\n", getpid(), ret);
		return -1; 
	}

	// OPERACJA NR 2
	ret=fifomutex_lock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Lock nr 2 - status: %d\n", getpid(), ret);
		return -1; 
	}

	usleep(200000); // 200 ms

	ret=fifomutex_unlock(&pd);
	if (ret < 0) {
		printf("PID = %d konczy sie. Unlock nr 2 - status: %d\n", getpid(), ret);
		return -1; 
	}

	return 0;
}
