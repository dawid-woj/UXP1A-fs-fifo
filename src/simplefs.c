#include "simplefs.h"
#include "simplefs_aux.h"
#include "sfs_ops.h"
#include "sfs_vd.h"
#include "fifo_mutex.h"
#include "desc_manager.h"
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***********************************************************************************************************************************************
 * SIMPLEFS MAKE 
 * Tworzy nowy plik SFS
 * Parametry:
 * name - nazwa tworzonego pliku SFS
 * blocks_per_inode - liczba blokow danych przypadajacych na pojedynczy inode
 * Zwraca:
 * SFS_OK - sukces, albo jeden z ponizszych kodow bledu:
 * SFS_VDFAULT, SFS_BAD_OPTION
 ***********************************************************************************************************************************************/
int simplefs_make(char* name, int blocks_per_inode)
{
  struct dir_header root_header;
  
  if(blocks_per_inode < 1)
  {
    return SFS_BAD_OPTION;
  }
  // Tworzy nowy plik sfs i otwiera go
  if(create_sfsfile(name, blocks_per_inode) < 0) 
  {
    return SFS_VDFAULT;
  }
  
  // Inicjacja danych katalogu root
  root_header.entries = 0;
  root_header.own_inode = SFS_ROOT_INODE;
  root_header.parent_dir = SFS_ROOT_INODE;
  write_to_block(SFS_ROOT_BLOCK, 0, (char*) &root_header, sizeof(root_header));
  
  // Zapisuje zmiany w mapach bitowych i superbloku oraz zamyka utworzony plik sfs
  close_sfsfile();
  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SIMPLEFS MOUNT
 * Przygotowuje witrualny dysk simplefs utworzony przez simplefs_make() do uzytku
 * Uruchamia demona fsinit
 * Zwraca
 * SFS_OK - sukces lub jeden z ponizszych kodow bledu:
 * SFS_EMOUNT, SFS_EINIT
 ***********************************************************************************************************************************************/
int simplefs_mount(char* name)
{
  if (fifomutex_startinit() < 0)
  {
    return SFS_EINIT;
  }

  if (link(name, SFS_LINK_NAME) < 0)
  {
    fifomutex_umount();
    return SFS_EMOUNT;
  }
  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SIMPLEFS UMOUNT
 * Odmontwuje witrualny dysk simplefs
 * Powoduje zakonczenie procesu fsinit
 * Zwraca:
 * SFS_OK - sukces
 ***********************************************************************************************************************************************/
int simplefs_umount(void)
{
  fifomutex_umount();
  unlink(SFS_LINK_NAME); // Usuniecie hardlinka
  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SIMPLEFS OPEN
 * Otwiera wskazany przez sciezke name plik w trybie mode
 * Mozliwe do wyspecyfikowania tryby otwarcia (mode):
 * SFS_RDONLY - plik otwierany w trybie do odczytu
 * SFS_WRONLY - plik otwierany w trybie do zapisu
 * SFS_RDWR - plik otwierany w trybie do zapisu i odczytu
 * SFS_CREAT - jesli plik nie istnieje to zostanie utworzony ze wskazanymi w mode prawami dostepu
 * Zwraca:
 * deskryptor otworzonego pliku, albo jeden z ponizszych kodow bledu:
 * SFS_BAD_OPTION, SFS_VDFAULT, SFS_TO_MANY_FILES, SFS_DIR_FULL, SFS_EACCESS, SFS_EOPENED,
 * SFS_EDESC, SFS_NAME_TO_LONG, SFS BAD_PATH, SFS_NOT_FOUND, SFS_BAD_NAME, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_open(char *name, int mode)
{
  int status;
  struct proc_data data;
  
  // Kontrola poprawnosci parametru mode - sprawdzamy czy podano tryb dostepu i znane flagi
  if(((mode & SFS_RDWR) == 0) || (mode & (~(SFS_RDWR | SFS_CREAT)))) 
  {
    return SFS_BAD_OPTION;
  }
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0) 
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_open(name, mode);	// Otwieranie pliku
  close_sfsfile();
  fifomutex_unlock(&data);
  
  return status; 			// Inode znalezionego pliku lub kod bledu
}

/***********************************************************************************************************************************************
 * SIMPLEFS UNLINK
 * Usuwa plik/katalog wskazany przez sciezke name z systemu simplefs
 * Zwraca:
 * SFS_OK - usuwanie zakonczone sukcesem lub jeden z ponizszych kodow bledow:
 * SFS_VDFAULT, SFS_NAME_TO_LONG, SFS BAD_PATH, SFS_NOT_FOUND, SFS_BAD_NAME, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_unlink(char *name)
{
  int status;
  struct proc_data data;
  
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0)
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_unlink(name);		// Usuwanie pliku/katalogu
  close_sfsfile();
  fifomutex_unlock(&data);
  return status;
}

/***********************************************************************************************************************************************
 * SIMPLEFS MKDIR
 * Tworzy katalog ze sciezka dostepu name
 * Zwraca:
 * SFS_OK - udalo sie utowrzyc katalog lub jeden z ponizszych kodow bledu:
 * SFS_EEXISTS, SFS_NAME_TO_LONG, SFS BAD_PATH, SFS_BAD_NAME, SFS_TO_MANY_FILES, SFS_NOSPACE, SFS_DIRFULL, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_mkdir(char *name)
{
  int status;
  struct proc_data data;
  
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0)
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_mkdir(name);		// Tworzenie katalogu
  close_sfsfile();
  fifomutex_unlock(&data);
  return status;
}

/***********************************************************************************************************************************************
 * SIMPLEFS CREAT
 * Tworzy nowy plik o sciezce dostepu name (o ile takowy nie istnieje) i otwiera go
 * Paramter mode okresla prawa dostepu do tworzonego pliku i jednoczesnie prawa z jakimi zostanie otworzony
 * Mozliwe do wyspecyfikowania tryby otwarcia (mode):
 * SFS_RDONLY - plik otwierany w trybie do odczytu
 * SFS_WRONLY - plik otwierany w trybie do zapisu
 * SFS_RDWR - plik otwierany w trybie do zapisu i odczytu
 * Zwraca:
 * deskryptor utworzonego/otworzonego pliku, albo jeden z ponizszych kodow bledu:
 * SFS_BAD_OPTION, SFS_VDFAULT, SFS_TO_MANY_FILES, SFS_DIR_FULL, SFS_EOPENED,
 * SFS_EDESC, SFS_NAME_TO_LONG, SFS BAD_PATH, SFS_BAD_NAME, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_creat (char *name, int mode)
{
  return simplefs_open(name, mode | SFS_CREAT);
}

/***********************************************************************************************************************************************
 * SIMPLEFS READ
 * Odczytuje z pliku o deskryptorze fd len bajtow do bufora bufora
 * Zwraca:
 * liczba odczytanych bajtow lub jeden z ponizszych kodow bledow:
 * SFS_BAD_DESC, SFS_VDFAULT, SFS_EDESC, SFS_EACCESS, SFS_EOF, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_read(int fd, char *buf, int len)
{
  int status;
  struct proc_data data;
  
  if(fd <= 0) 
  {
    return SFS_BAD_DESC;		// Niepoprawny deskryptor
  }
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0)
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_read(fd, buf, len);	// Odczyt
  close_sfsfile();
  fifomutex_unlock(&data);
  return status;			// Liczba odczytanych bajtow lub kod bledu
}

/***********************************************************************************************************************************************
 * SIMPLEFS WRITE
 * Zapisuje do pliku o deskryptorze fd len bajtow z bufora bufora
 * Zwraca:
 * liczba zapisanych bajtow lub jeden z ponizszych kodow bledow:
 * SFS_BAD_DESC, SFS_VDFAULT, SFS_EDESC, SFS_EACCESS, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_write(int fd, char *buf, int len)
{
  int status;
  struct proc_data data;
  
  if(fd <= 0) 
  {
    return SFS_BAD_DESC;		// Niepoprawny deskryptor
  }
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0)
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_write(fd, buf, len);	// Zapis
  close_sfsfile();
  fifomutex_unlock(&data);
  return status;			// Liczba zapisanych bajtow lub kod bledu
}

/***********************************************************************************************************************************************
 * SIMPLEFS LSEEK
 * Ustawia wskaznik odczytu/zapisu pliku dla procesu na wartosc offset wzgledem parametru whence
 * Mozliwe wartosci whence
 * SFS_SEEK_CUR - offset jest dodawany do obecnej pozycji wskaznika odczytu/zapisu
 * SFS_SEEK_SET - offset jest ustawiany wzgledem poczatku pliku
 * SFS_SEEK_END - offset jest ustawiany wzgledem konca pliku
 * Zwraca:
 * aktualna pozycje w pliku lub jeden z ponizszych kodow bledu:
 * SFS_BAD_DESC, SFS_VDFAULT, SFS_EDESC, SFS_BAD_OPTION, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_lseek(int fd, int whence, int offset)
{
  int status;
  struct proc_data data;
  
  if(fd <= 0) 
  {
    return SFS_BAD_DESC;		// Niepoprawny deskryptor
  }
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  if (open_sfsfile() < 0)
  {
    fifomutex_unlock(&data);
    return SFS_VDFAULT; 		// Blad dostepu do pliku sfs 
  }
  status = sfsop_lseek(fd, whence, offset);	// Ustalenie nowej pozycji w pliku
  close_sfsfile();
  fifomutex_unlock(&data);
  return status;			// Nowa pozycja odczytu/zapisu lub kod bledu
}

/***********************************************************************************************************************************************
 * SIMPLEFS CLOSE
 * Zamyka plik o deskryptorze fd
 * Zwraca:
 * SFS_OK - sukces, albo jeden z ponizszych kodow bledu:
 * SFS_BAD_DESC, SFS_ECLOSED, SFS_EDESC, SFS_ESYNC
 ***********************************************************************************************************************************************/
int simplefs_close(int fd)
{
  int status;
  struct proc_data data;
  
  if(fd <= 0) 
  {
    return SFS_BAD_DESC;		// Niepoprawny deskryptor
  }
  if(fifomutex_lock(&data) < 0)
  {
    return SFS_ESYNC;			// Natrafiono na blad przed otrzymaniem tokena
  }
  status = sfsop_close(fd);		// Ustalenie nowej pozycji w pliku
  fifomutex_unlock(&data);
  return status;			// Nowa pozycja odczytu/zapisu lub kod bledu
}
/***********************************************************************************************************************************************
 * KONIEC
 ***********************************************************************************************************************************************/
