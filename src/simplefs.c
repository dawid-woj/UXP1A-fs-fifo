#include "simplefs.h"
#include "simplefs_aux.h"
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
 * @todo: opisac zwracane wartosci
 * @todo: kontrola poprawnosci
 ***********************************************************************************************************************************************/
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
  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SIMPLEFS MOUNT
 * Przygotowuje witrualny dysk simplefs utworzony przez simplefs_make() do uzytku
 * Uruchamia demona fsinit
 * @todo: opisac zwracane wartosci
 * @todo: kontrola poprawnosci
 * @todo: dodac obsluge procesu fsinit
 ***********************************************************************************************************************************************/
int simplefs_mount(char* name)
{
  // Utworzenie fsinit
  link(name, SFS_LINK_NAME); // Nie wiem czy nie przeniesc tego do procesu fsinit
  return 0;
}

/***********************************************************************************************************************************************
 * SIMPLEFS UMOUNT
 * Odmontwuje witrualny dysk simplefs
 * Powoduje zakonczenie procesu fsinit
 * @todo: opisac zwracane wartosci
 * @todo: kontrola poprawnosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_umount(void)
{
  unlink(SFS_LINK_NAME); // Nie wiem czy nie przeniesc tego do procesu fsinit
  // Zakonczenie fsinit
  // fifomutex_unmount();
  return 0;
}

/***********************************************************************************************************************************************
 * SIMPLEFS OPEN
 * Otwiera wskazany przez sciezke name plik w trybie mode
 * Mozliwe do wyspecyfikowania tryby otwarcia (mode):
 * SFS_RDONLY - plik otwierany w trybie do odczytu
 * SFS_WRONLY - plik otwierany w trybie do zapisu
 * SFS_RDWR - plik otwierany w trybie do zapisu i odczytu
 * SFS_CREAT - jesli plik nie istnieje to zostanie utworzony ze wskazanymi w mode prawami dostepu
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_open(char *name, int mode)
{
  struct directory dir;
  struct inode fileinode;
  int fileinodeid;
  char* filename;
  char omode = mode & (~SFS_CREAT); // Maska z wygaszonym bitem CREAT
  int status = SFS_OK;
  
  // Kontrola poprawnosci parametru mode
  if(((omode & SFS_RDWR) == 0) || (mode & (~(SFS_RDWR | SFS_CREAT)))) // Nie wyspecyfikowano trybu dostepu albo nieznane flagi
  {
    return SFS_BAD_OPTION;
  }
  
  // fifomutex_lock();
  open_sfsfile();
  fileinodeid = get_objects_inode_and_directory(name, &filename, SFS_FILE, &dir);
  if(fileinodeid < 0) // Nie znaleziono szukanego pliku lub niepoprawna sciezka
  {
    // Nie znaleziono pliku i ustawiona flaga CREAT
    if((fileinodeid == SFS_NOT_FOUND) && (mode & SFS_CREAT))
    {
      fileinodeid = reserve_inode(); // Rezerwujemy nowy inod
      if(fileinodeid == SFS_NO_INODE) // Brak wolnych inodow
      {
	status = SFS_TO_MANY_FILES;
      }
      else if (add_to_dir(fileinodeid, SFS_FILE, filename, &dir) == SFS_DIR_FULL) // Dodajemy nowy wpis do katalogu
      {
	status = SFS_DIR_FULL;
	free_inode(fileinodeid);
      }
      else // Dodanie wpisu katalogu i rezerwacja inoda sie powiodly
      {
	close_dir(&dir); // Zapisujemy zmiany w katalogu i przygotowujemy zawartosc inode
	fileinode.filetype = SFS_FILE;
	fileinode.mode = omode;	
	fileinode.nblocks = 0;
	fileinode.filesize = 0;
	set_inode(fileinodeid, &fileinode);
      }
    }
    else
    {
      status = fileinodeid; // Zapamietujemy kod bledu get_objects_inode_and_directory()
    }
  }
  else // Plik istenieje - trzeba sprawdzic prawa dostepu
  {
    get_inode(fileinodeid, &fileinode);
    if((omode & fileinode.mode) != omode)
    {
      status = SFS_EACCESS;
    }
  }
  
  // Dodanie deskryptora w pliku tymczasowym
  if(status == SFS_OK)
  {
    int tmp = add_desc(fileinodeid, omode);
    if(tmp == -1)
    {
      status = SFS_EOPENED;
    }
    else if(tmp < 0)
    {
      status = SFS_EDESC;
    }
  }
  
  close_sfsfile();
  //fifomutex_unlock();
  
  return ((status == SFS_OK) ? (fileinodeid) : (status));
}

/***********************************************************************************************************************************************
 * SIMPLEFS UNLINK
 * Usuwa plik/katalog wskazany przez sciezke name z systemu simplefs
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_unlink(char *name)
{
  struct directory dir;
  int inodeid;
  char* objname;
  
  // fifomutex_lock();
  open_sfsfile();
  // Parsowanie ścieżki i przejście po katalogach
  inodeid = get_objects_inode_and_directory(name, &objname, SFS_DIRECTORY + SFS_FILE, &dir);
  if(inodeid > 0) // Znaleziono szukany obiekt
  {
    if(dir.entries[dir.found_object_position].type == SFS_FILE) // Usuwamy plik
    {
      delete_file(inodeid);
    }
    else
    {
      delete_directory(inodeid);
    }
    del_from_dir(&dir); // Usuwamy wpis z katalogu zawierajacego obiekt
    close_dir(&dir); // Zapisujemy zmiany w katalogu
    inodeid = SFS_OK;
  }
  close_sfsfile();
  //fifomutex_unlock();
  return inodeid;
}

/***********************************************************************************************************************************************
 * SIMPLEFS MKDIR
 * Tworzy katalog ze sciezka dostepu name
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_mkdir(char *name)
{
  // fifomutex_lock();
  open_sfsfile();

  // Rezerwujemy dla niego blok danych i zapisujemy go odpowiednią zawartością 
  // (nazwa własna katogu, nazwa katalogu nadrzędnego, wskazania na ich inody) → reserve_block(), write_to_block()
  // Dodajemy wpis do katalogu zawierającego dodawany obiekt
  struct directory dir;
  struct inode dirinode;
  int dirinodeid;
  char* dirname;
  int status = SFS_OK;
  unsigned short new_dir_block;
  
  // fifomutex_lock();
  open_sfsfile();
  
  dirinodeid = get_objects_inode_and_directory(name, &dirname, SFS_DIRECTORY, &dir);
  if(dirinodeid < 0) // Nie znaleziono szukanego katalogu lub niepoprawna sciezka
  {
    // Nie znaleziono katalogu -> trzeba utworzyc
    if(dirinodeid == SFS_NOT_FOUND)
    {
      dirinodeid = reserve_inode(); // Rezerwujemy nowy inod
      if(dirinodeid == SFS_NO_INODE) // Brak wolnych inodow
      {
	status = SFS_TO_MANY_FILES;
      }
      else if((new_dir_block = reserve_block()) == SFS_NO_BLOCK)
      {
	status = SFS_NOSPACE;
	free_inode(dirinodeid);
      }
      else if (add_to_dir(dirinodeid, SFS_DIRECTORY, dirname, &dir) == SFS_DIR_FULL) // Dodajemy nowy wpis do katalogu
      {
	status = SFS_DIR_FULL;
	free_inode(dirinodeid);
	free_block(new_dir_block);
      }
      else // Dodanie wpisu katalogu i rezerwacja inoda sie powiodly
      {
	struct dir_header new_dir_head;
	
	// Zapisujemy zmiany w katalogu nadrzednym
	close_dir(&dir); 
	
	//Przygotowujemy zawartosc inode nowego katalogu
	dirinode.filetype = SFS_DIRECTORY;
	dirinode.mode = SFS_RDWR; // Co robimy z prawami dostepu do katalogu ???
	dirinode.nblocks = 1;
	dirinode.filesize = SFS_BLOCK_SIZE;
	dirinode.blocks[0] = new_dir_block;
	set_inode(dirinodeid, &dirinode);
	
	// Przygotowujemy zawartosc bloku danych nowego katalogu
	new_dir_head.entries = 0;
	new_dir_head.own_inode = dirinodeid;
	new_dir_head.parent_dir = dir.head.own_inode;
	write_to_block(dirinode.blocks[0], 0, (char*)&new_dir_head, sizeof(struct dir_header));
      }
    }
    else
    {
      status = dirinodeid; // Zapamietujemy kod bledu get_objects_inode_and_directory()
    }
  }
  else // Katalog istenieje
  {
    status = SFS_EEXISTS;
  }

  close_sfsfile();
  //fifomutex_unlock();
  
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
 * @todo: opisac zwracane wartosci
 ***********************************************************************************************************************************************/
int simplefs_creat (char *name, int mode)
{
  return simplefs_open(name, mode | SFS_CREAT);
}

/***********************************************************************************************************************************************
 * SIMPLEFS READ
 * Odczytuje z pliku o deskryptorze fd len bajtow do bufora bufora
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_read(int fd, char *buf, int len)
{
  int readbytes = 0;
  int offset;
  char mode;
  struct inode inod;
  
  if(fd <= 0) return SFS_BAD_DESC;
  
  // fifomutex_lock();
  open_sfsfile();
  get_inode(fd, &inod); // Wczytanie inode
  // Otwieramy plik tymczasowy i wczytujemy prawa dostepu oraz offset wskaznika odczytu
  if(get_desc(fd, &offset, &mode)) // Blad pliku dostepu do pliku tymczasowego
  {
    readbytes = SFS_EDESC;
  }
  else if((mode | SFS_RDONLY) == 0) // Nie mozna czytac z pliku
  {
    readbytes = SFS_EACCESS;
  }
  else if(offset >= inod.filesize) // Koniec pliku
  {
    readbytes = SFS_EOF;
  }
  else // Jak do tej pory wszystko OK
  {
    int readblock;
    int offset_in_block;
    int tmp;
    
    // Wyznaczenie nr bloku i offsetu w bloku
    readblock = offset / SFS_BLOCK_SIZE;
    offset_in_block = offset % SFS_BLOCK_SIZE;
    
    // Odczyt z blokow adresowanych bezposrednio
    while(len > 0)
    {
      if(readblock >= SFS_DIRECT_BLOCKS) break; // Odczytano wszysktkie bloki adresowane bezposrednio
      tmp = read_from_block(inod.blocks[readblock], offset_in_block, buf+readbytes, len);
      readbytes += tmp;
      len -= tmp;
      ++readblock;
      offset_in_block = 0;
    }
    if(len > 0) // Trzeba odczytac bloki adresowane posrednio
    {
      unsigned short indirectblocks[inod.nblocks - SFS_DIRECT_BLOCKS];
      
      read_from_block(inod.blocks[SFS_INDIRECT_POINTER], 0, (char*)indirectblocks, sizeof(unsigned short) * (inod.nblocks - SFS_DIRECT_BLOCKS));
      readblock = 0;
      while(len > 0)
      {
	if(readblock >= SFS_INDIRECT_BLOCKS) break; // Odczytano wszysktkie bloki adresowane posrednio
	tmp = read_from_block(indirectblocks[readblock], offset_in_block, buf+readbytes, len);
	readbytes += tmp;
	len -= tmp;
	++readblock;
	offset_in_block = 0;
      }
    }
  }
  update_desc(fd, offset + readbytes);
  close_sfsfile();
  //fifomutex_unlock();
  return readbytes;
}

/***********************************************************************************************************************************************
 * SIMPLEFS WRITE
 * Zapisuje do pliku o deskryptorze fd len bajtow z bufora bufora
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_write(int fd, char *buf, int len)
{
  int writtenbytes = 0;
  int offset;
  int new_offset;
  char mode;
  struct inode inod;
  
  if(fd <= 0) return SFS_BAD_DESC;
  
  // fifomutex_lock();
  open_sfsfile();
  get_inode(fd, &inod); // Wczytanie inode
  // Otwieramy plik tymczasowy i wczytujemy prawa dostepu oraz offset wskaznika zapisu
  if(get_desc(fd, &offset, &mode)) // Blad pliku dostepu do pliku tymczasowego
  {
    writtenbytes = SFS_EDESC;
  }
  else if((mode | SFS_WRONLY) == 0) // Nie mozna pisac do pliku
  {
    writtenbytes = SFS_EACCESS;
  }
  else // Jak do tej pory wszystko OK
  {
    int writeblock;
    int offset_in_block;
    int tmp;
    
    if(offset > inod.filesize) // Wskaznik zapisu wskazuje poza koniec pliku
    {
      offset = inod.filesize;
    }
    
    // Wyznaczenie nr bloku i offsetu w bloku
    writeblock = offset / SFS_BLOCK_SIZE;
    offset_in_block = offset % SFS_BLOCK_SIZE;
    
    // Zapis do blokow adresowanych bezposrednio
    while(len > 0)
    {
      if(writeblock >= SFS_DIRECT_BLOCKS) break; // Zapisano wszysktkie bloki adresowane bezposrednio
      if(writeblock >= inod.nblocks) // Potrzebny nowy blok danych
      {
	inod.blocks[writeblock] = reserve_block();
	if(inod.blocks[writeblock] == SFS_NO_BLOCK) break; // Nie ma wolnych blokow
	++inod.nblocks;
      }
      tmp = write_to_block(inod.blocks[writeblock], offset_in_block, buf+writtenbytes, len);
      writtenbytes += tmp;
      len -= tmp;
      ++writeblock;
      offset_in_block = 0;
    }
    if(len > 0) // Trzeba odczytac bloki adresowane posrednio
    {
      unsigned short indirectblocks[SFS_INDIRECT_BLOCKS];
      
      if(inod.nblocks <= SFS_DIRECT_BLOCKS) // Bloki bezposrednie nie byly wczesniej wykorzystywane
      {
	if ((inod.blocks[SFS_INDIRECT_POINTER] = reserve_block()) == SFS_NO_BLOCK)
	{
	  len = 0; // Nie mozna kontynuowac zapisu - brak miejsca na blok posredni
	}
      }
      read_from_block(inod.blocks[SFS_INDIRECT_POINTER], 0, (char*)indirectblocks, sizeof(unsigned short) * SFS_INDIRECT_BLOCKS);
      writeblock = 0;
      while(len > 0)
      {
	if(writeblock >= SFS_INDIRECT_BLOCKS) break; // Zapisano wszysktkie bloki adresowane posrednio
	if(writeblock + SFS_DIRECT_BLOCKS >= inod.nblocks) // Potrzebny nowy blok danych
	{
	  indirectblocks[writeblock] = reserve_block();
	  if(indirectblocks[writeblock] == SFS_NO_BLOCK) break; // Nie ma wolnych blokow
	  ++inod.nblocks;
	}
	tmp = write_to_block(indirectblocks[writeblock], offset_in_block, buf+writtenbytes, len);
	writtenbytes += tmp;
	len -= tmp;
	++writeblock;
	offset_in_block = 0;
      }
      // Zapisanie zmain w blokach adresowanych bezposrednio
      if(inod.blocks[SFS_INDIRECT_POINTER] != SFS_NO_BLOCK)
      {
	write_to_block(inod.blocks[SFS_INDIRECT_POINTER], 0, (char*)indirectblocks, sizeof(unsigned short) * SFS_INDIRECT_BLOCKS);
      }
    }
  }
  new_offset = offset + writtenbytes;
  update_desc(fd, new_offset);
  inod.filesize = (inod.filesize > new_offset) ? (inod.filesize) : (new_offset);
  set_inode(fd, &inod);
  close_sfsfile();
  //fifomutex_unlock();
  return writtenbytes;
}

/***********************************************************************************************************************************************
 * SIMPLEFS LSEEK
 * Ustawia wskaznik odczytu/zapisu pliku dla procesu na wartosc offset wzgledem parametru whence
 * Mozliwe wartosci whence
 * SFS_SEEK_CUR - offset jest dodawany do obecnej pozycji wskaznika odczytu/zapisu
 * SFS_SEEK_SET - offset jest ustawiany wzgledem poczatku pliku
 * SFS_SEEK_END - offset jest ustawiany wzgledem konca pliku
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_lseek(int fd, int whence, int offset)
{
  int rwpointer, retval;
  char mode;
  
  if(fd <= 0) return SFS_BAD_DESC;

  // fifomutex_lock();
  // Otwórz plik tymczasowy i wyciągnij z niego wskaźnik odczytu/zapisu → desc_get()
 
  if(get_desc(fd, &rwpointer, &mode)) // Blad pliku dostepu do pliku tymczasowego
  {
    retval = SFS_EDESC;
  }
  else
  {
    struct inode inod;
    
    switch (whence)
    {
      case SFS_SEEK_SET:
	rwpointer = offset;
	retval = rwpointer;
	break;
      case SFS_SEEK_CUR:
	rwpointer += offset;
	retval = rwpointer;
	break;
      case SFS_SEEK_END:
	open_sfsfile();
	get_inode(fd, &inod);
	rwpointer = inod.filesize + offset;
	close_sfsfile();
	retval = rwpointer;
	break;
      default:
	retval = SFS_BAD_OPTION;
    }
    update_desc(fd, rwpointer);
  }
  //fifomutex_unlock();
  return retval; // retval = rwpointer lub kod bledu
}

/***********************************************************************************************************************************************
 * SIMPLEFS CLOSE
 * Zamyka plik o deskryptorze fd
 * @todo: opisac zwracane wartosci
 * @todo: dodac synchronizacje z token ring
 ***********************************************************************************************************************************************/
int simplefs_close(int fd)
{
  int retval;
  
  // fifomutex_lock();
  open_sfsfile();
  // Usuń z pliku tymczasowego wpis danego procesu
  retval = del_desc(fd);
  if(retval) // Wystapil jakis blad
  {
    if(retval == -1)
    {
      retval = SFS_ECLOSED;
    }
    else
    {
      retval = SFS_EDESC;
    }
  }
  else
  {
    retval = SFS_OK;
  }
  close_sfsfile();
  //fifomutex_unlock();
  return retval;
}

/***********************************************************************************************************************************************
 * KONIEC
 ***********************************************************************************************************************************************/
