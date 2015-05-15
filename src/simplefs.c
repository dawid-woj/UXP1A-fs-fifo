#include "simplefs.h"
#include "sfs_vd.h"
#include "fifo_mutex.h"
#include "desc_manager.h"

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

struct dir_entry
{
  int inode_number;	// Nr inode pliku
  char type;		// Typ wpisu (plik/katalog)
  char filename[59];	// Nazwa wpisu (zakonczona 0)
};

/***************************************************************************
 * Funkcje pomocnicze
 **************************************************************************/

int find_in_directory(int dirinodeid, char* name, char type)
{
  return 0;
}

int get_objects_directory(char* path)
{
  return 0;
}

/***************************************************************************
 * API modulu
 **************************************************************************/

int simplefs_make(char* name, int inodes)
{
  return 0;
}

int simplefs_mount(char* name)
{
  return 0;
}

int simplefs_umount(void)
{
  return 0;
}

int simplefs_open(char *name, int mode)
{
  return 0;
}

int simplefs_unlink(char *name)
{
  return 0;
}

int simplefs_mkdir(char *name)
{
  return 0;
}

int simplefs_creat (char *name, int mode)
{
  return 0;
}

int simplefs_read(int fd, char *buf, int len)
{
  return 0;
}

int simplefs_write(int fd, char *buf, int len)
{
  return 0;
}

int simplefs_lseek(int fd, int whence, int offset)
{
  return 0;
}

int simplefs_close(int fd)
{
  return 0;
}