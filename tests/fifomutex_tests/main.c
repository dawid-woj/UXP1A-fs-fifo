#include "../../src/fifo_mutex.h"

int main()
{	
	struct proc_data pd;
	signal(SIGPIPE, SIG_IGN);
	fifomutex_lock(&pd);
	sleep(2);
	fifomutex_unlock(&pd);
	return 0;
}
