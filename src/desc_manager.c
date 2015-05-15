#include "desc_manager.h"

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

struct fd_entry
{
  int owner_pid;	// Pid właściciela deskryptora
  int rwoffset;		// Pozycja wskaźnika zapisu/odczytu
  char mode;		// Tryb w którym otworzono plik
};

/***************************************************************************
 * API modulu
 **************************************************************************/

int add_desc(int fd, char mode)
{
  return 0;
}

int del_desc(int fd)
{
  return 0;
}

int update_desc(int fd, int offset)
{
  return 0;
}

int get_desc(int fd, int* offset, char* mode)
{
  return 0;
}

int del_descfile(int fd)
{
  return 0;
}

