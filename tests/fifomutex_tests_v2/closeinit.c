#include "../../src/fifo_mutex.h"

int main()
{	
	int cnt=0;
	while(fifomutex_umount()!=0)
	{
		if(cnt>4)
			break;
		cnt++;
		sleep(1);
	}
	return 0;
}
