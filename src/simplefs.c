#include "simplefs.h"
#include "sfs_vd.h"
#include "fifo_mutex.h"
#include "desc_manager.h"
#include <unistd.h>

#define MAX_DIR_ENTRIES	127 // Masksymalna liczba wspisow katalogu (nie liczac naglowka katalogu)

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

/* Pozycja katalogu */
struct dir_entry
{
  int inode_number;	// Nr inode pliku
  char type;		// Typ wpisu (plik/katalog)
  char filename[27];	// Nazwa wpisu (zakonczona 0)
};

/* Naglowek katalogu */
struct dir_header
{
  int entries;		// Liczba pozycji katalogu (podkatalogi + pliki)
  int own_inode;	// Wlasny nr inoda (odwolania typu: .)
  int parent_dir;	// Nr inoda katalogu nadrzednego (odwolania typu: ..)
  char padding[20];	// Nieuzywane
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

/* Tworzy nowy plik SFS
 * Parametry:
 * name - nazwa tworzonego pliku SFS
 * blocks_per_inode - liczba blokow danych przypadajacych na pojedynczy inode
 * Na razie nie ma jeszcze sprawdzania poprawnosci wykoniania ???
 */
int simplefs_make(char* name, int blocks_per_inode)
{
  struct dir_header root_header;
  
  create_sfsfile(name, blocks_per_inode); // Tworzy nowy plik sfs i otwiera go
  
  // Inicjacja danych katalogu root
  root_header.entries = 0;
  root_header.own_inode = SFS_ROOT_INODE;
  root_header.parent_dir = SFS_ROOT_INODE;
  write_to_block(SFS_ROOT_BLOCK, 0, (char*) &root_header, sizeof(root_header));
  
  close_sfsfile(); // Zapisuje zmiany w mapach bitowych i superbloku oraz zamyka utworzony plik sfs
  return 0;
}

int simplefs_mount(char* name)
{
  link(name, SFS_LINK_NAME); // Nie wiem czy nie przeniesc tego do procesu fsinit
  return 0;
}

int simplefs_umount(void)
{
  unlink(SFS_LINK_NAME); // Nie wiem czy nie przeniesc tego do procesu fsinit
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