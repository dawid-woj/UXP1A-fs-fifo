#include "../../src/fifo_mutex.h"

int main()
{	
	if(fifomutex_umount()!=0)
	{
		puts("Nie mozna zakmnkac inita - nie istnieje fifoinit");
		return -1;
	}
	return 0;
}
