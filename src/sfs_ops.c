#include "simplefs.h"
#include "simplefs_aux.h"
#include "sfs_vd.h"
#include "desc_manager.h"

/***********************************************************************************************************************************************
 * SFSOP OPEN
 * Otwiera wskazany przez sciezke name plik w trybie mode
 * Mozliwe do wyspecyfikowania tryby otwarcia (mode):
 * SFS_RDONLY - plik otwierany w trybie do odczytu
 * SFS_WRONLY - plik otwierany w trybie do zapisu
 * SFS_RDWR - plik otwierany w trybie do zapisu i odczytu
 * SFS_CREAT - jesli plik nie istnieje to zostanie utworzony ze wskazanymi w mode prawami dostepu
 * Zwraca deskryptor otworzonego pliku lub odpowiedni kod bledu:
 * SFS_TO_MANY_FILES
 * SFS_DIR_FULL
 * SFS_EACCESS
 * SFS_EOPENED
 * SFS_EDESC
 * SFS_NAME_TO_LONG
 * SFS BAD_PATH
 * SFS_NOT_FOUND
 * SFS_BAD_NAME
 ***********************************************************************************************************************************************/
int sfsop_open(char *name, int mode)
{
  struct directory dir;
  struct inode fileinode;
  int fileinodeid;
  char* filename;
  char omode = mode & (~SFS_CREAT); 		// Maska z wygaszonym bitem CREAT
  
  int tmp;
 
  fileinodeid = lookup(name, &filename, SFS_FILE, &dir);
  
  // Przypadek 1: Plik nie istnieje  i ustawiona flaga CREAT -> tworzymy nowy plik
  if((fileinodeid == SFS_NOT_FOUND) && (mode & SFS_CREAT))
  {
    fileinodeid = reserve_inode(); 		// Rezerwujemy nowy inod
    if(fileinodeid == SFS_NO_INODE)
    {
      return SFS_TO_MANY_FILES;			// Brak wolnych inodow
    }
    if(add_to_dir(fileinodeid, SFS_FILE, filename, &dir) == SFS_DIR_FULL) // Dodajemy nowy wpis do katalogu
    {
      free_inode(fileinodeid);
      return SFS_DIR_FULL;			// Nie ma miejsca na nowy wpis w katalogu
    }
    // Zapisujemy zmiany w katalogu i przygotowujemy zawartosc inode
    close_dir(&dir); 			
    fileinode.filetype = SFS_FILE;
    fileinode.mode = omode;	
    fileinode.nblocks = 0;
    fileinode.filesize = 0;
    set_inode(fileinodeid, &fileinode);
  }
  // Przypadek 2: Nie znaleziono pliku
  else if(fileinodeid < 0)
  {
    return fileinodeid; 			// fileinodeid zawiera kod bledu
  }
  // Przypadek 3: Plik istnieje - trzeba sprawdzic prawa dostepu
  else 
  {
    get_inode(fileinodeid, &fileinode);
    if((omode & fileinode.mode) != omode)
    {
      return SFS_EACCESS;			// Niepoprawny tryb dostepu
    }
  }
  
  // Dodanie deskryptora w pliku tymczasowym
  tmp = add_desc(fileinodeid, omode);
  if(tmp == -1)
  {
    return SFS_EOPENED;				// Plik jest juz otwarty
  }
  else if(tmp < 0)
  {
    return SFS_EDESC;				// Blad dostepu do pliku tymczasowego
  }
  return fileinodeid;
}

/***********************************************************************************************************************************************
 * SFSOP UNLINK
 * Usuwa plik/katalog wskazany przez sciezke name z systemu simplefs
 * Zwraca:
 * SFS_OK
 * SFS_NAME_TO_LONG
 * SFS BAD_PATH
 * SFS_NOT_FOUND
 * SFS_BAD_NAME
 ***********************************************************************************************************************************************/
int sfsop_unlink(char *name)
{
  struct directory dir;
  int inodeid;
  char* objname;
  
  // Parsowanie ścieżki i przejście po katalogach
  inodeid = lookup(name, &objname, SFS_DIRECTORY + SFS_FILE, &dir);
  
  if(inodeid < 0)
  {
    return inodeid;			// Nie znaleziono obiektu, zwroc kod bledu
  }
  if(dir.entries[dir.found_object_position].type == SFS_FILE)
  {
    delete_file(inodeid);		// Usuwany obiekt jest plikiem
  }
  else
  {
    delete_directory(inodeid);		// Usuwany obiekt jest katalogiem
  }
  del_from_dir(&dir); 			// Usuwamy wpis z katalogu zawierajacego obiekt
  close_dir(&dir); 			// Zapisujemy zmiany w katalogu

  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SFSOP MKDIR
 * Tworzy katalog ze sciezka dostepu name
 * Zwraca:
 * SFS_EEXISTS
 * SFS_NAME_TO_LONG
 * SFS BAD_PATH
 * SFS_BAD_NAME
 * SFS_TO_MANY_FILES
 * SFS_NOSPACE
 * SFS_DIRFULL
 * SFS_OK
 ***********************************************************************************************************************************************/
int sfsop_mkdir(char *name)
{
  struct directory dir;
  struct inode dirinode;
  int dirinodeid;
  char* dirname;
  unsigned short new_dir_block;
  struct dir_header new_dir_head;
  
  dirinodeid = lookup(name, &dirname, SFS_DIRECTORY, &dir);
  if(dirinodeid > 0)
  {
    return SFS_EEXISTS;			// Katalog juz istnieje
  }
  if(dirinodeid != SFS_NOT_FOUND)
  {
    return dirinodeid;			// Zwracamy kod bledu wynikajacy z niepoprawnej sciezki
  }
  // Tworzymy nowy katalog
  dirinodeid = reserve_inode(); 	// Rezerwujemy nowy inod
  if(dirinodeid == SFS_NO_INODE) 	// Brak wolnych inodow
  {
    return SFS_TO_MANY_FILES;
  }
  new_dir_block = reserve_block();	// Rezerwujemy blok na zawartosc katalogu
  if(new_dir_block == SFS_NO_BLOCK)
  {
      free_inode(dirinodeid);
      return SFS_NOSPACE;		// Brak wolnych blokow
  }
  if(add_to_dir(dirinodeid, SFS_DIRECTORY, dirname, &dir) == SFS_DIR_FULL) // Dodajemy nowy wpis do katalogu
  {
    free_inode(dirinodeid);
    free_block(new_dir_block);
    return SFS_DIR_FULL;			// Brak miejsca w katalogu
   }
  //Dodanie wpisu katalogu i rezerwacja inoda sie powiodly
  close_dir(&dir); 			// Zapisujemy zmiany w katalogu nadrzednym
	
  //Przygotowujemy zawartosc inode nowego katalogu
  dirinode.filetype = SFS_DIRECTORY;
  dirinode.mode = SFS_RDONLY;
  dirinode.nblocks = 1;
  dirinode.filesize = SFS_BLOCK_SIZE;
  dirinode.blocks[0] = new_dir_block;
  set_inode(dirinodeid, &dirinode);
	
  // Przygotowujemy zawartosc bloku danych nowego katalogu
  new_dir_head.entries = 0;
  new_dir_head.own_inode = dirinodeid;
  new_dir_head.parent_dir = dir.head.own_inode;
  write_to_block(dirinode.blocks[0], 0, (char*)&new_dir_head, sizeof(struct dir_header));
  
  return SFS_OK;
}

/***********************************************************************************************************************************************
 * SFSOP READ
 * Odczytuje z pliku o deskryptorze fd len bajtow do bufora bufora
 * Zwaraca liczbe odczytanych bajtow lub kod bledu:
 * SFS_EDESC
 * SFS_EACCESS
 * SFS_EOF
 ***********************************************************************************************************************************************/
int sfsop_read(int fd, char *buf, int len)
{
  int readbytes = 0;
  int offset;
  char mode;
  int readblock;
  int offset_in_block;
  int tmp;
  struct inode inod;
  
  get_inode(fd, &inod); 		// Wczytanie inode
  // Otwieramy plik tymczasowy i wczytujemy prawa dostepu oraz offset wskaznika odczytu
  if(get_desc(fd, &offset, &mode)) 
  {
    return SFS_EDESC;			// Blad pliku dostepu do pliku tymczasowego
  }
  if((mode | SFS_RDONLY) == 0) 
  {
    readbytes = SFS_EACCESS;		// Nie mozna czytac z pliku
  }
  if(offset >= inod.filesize) 		
  {
    return SFS_EOF;			// Pozyca odczytu wskazuje poza koniec pliku
  }
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
  if(len > 0) 				// Trzeba odczytac bloki adresowane posrednio
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
  update_desc(fd, offset + readbytes);	// Zapisujemy nowy offset w pliku
  return readbytes;
}

/***********************************************************************************************************************************************
 * SFSOP WRITE
 * Zapisuje do pliku o deskryptorze fd len bajtow z bufora bufora
 * Zwaraca liczbe zapisanych bajtow lub kod bledu:
 * SFS_EDESC
 * SFS_EACCESS
 ***********************************************************************************************************************************************/
int sfsop_write(int fd, char *buf, int len)
{
  int writtenbytes = 0;
  int offset;
  int new_offset;
  int writeblock;
  int offset_in_block;
  int tmp;
  char mode;
  struct inode inod;
  
  get_inode(fd, &inod); 		// Wczytanie inode
  // Otwieramy plik tymczasowy i wczytujemy prawa dostepu oraz offset wskaznika zapisu
  if(get_desc(fd, &offset, &mode)) 	
  {
    return SFS_EDESC;			// Blad pliku dostepu do pliku tymczasowego
  }
  if((mode | SFS_WRONLY) == 0) 
  {
    return SFS_EACCESS;			// Nie mozna pisac do pliku
  }
  if(offset > inod.filesize) 		// Wskaznik zapisu wskazuje poza koniec pliku
  {
    offset = inod.filesize;		// Nie zachowujemy ciaglosc pliku
  }
    
  // Wyznaczenie nr bloku i offsetu w bloku
  writeblock = offset / SFS_BLOCK_SIZE;
  offset_in_block = offset % SFS_BLOCK_SIZE;
    
  // Zapis do blokow adresowanych bezposrednio
  while(len > 0)
  {
    if(writeblock >= SFS_DIRECT_BLOCKS) break; // Zapisano wszysktkie bloki adresowane bezposrednio
    if(writeblock >= inod.nblocks) 	// Potrzebny nowy blok danych
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
  if(len > 0) 				// Trzeba zapisac bloki adresowane posrednio
  {
    unsigned short indirectblocks[SFS_INDIRECT_BLOCKS];
      
    if(inod.nblocks <= SFS_DIRECT_BLOCKS) // Bloki bezposrednie nie byly wczesniej wykorzystywane
    {
      if ((inod.blocks[SFS_INDIRECT_POINTER] = reserve_block()) == SFS_NO_BLOCK)
      {
	  len = 0; 			// Nie mozna kontynuowac zapisu - brak miejsca na blok posredni
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
  
  new_offset = offset + writtenbytes;
  update_desc(fd, new_offset);	// Zapisujemy nowy offset w pliku
  inod.filesize = (inod.filesize > new_offset) ? (inod.filesize) : (new_offset); // Aktualizacja rozmiaru pliku
  set_inode(fd, &inod);
  return writtenbytes;
}

/***********************************************************************************************************************************************
 * SFSOP LSEEK
 * Ustawia wskaznik odczytu/zapisu pliku dla procesu na wartosc offset wzgledem parametru whence
 * Mozliwe wartosci whence
 * SFS_SEEK_CUR - offset jest dodawany do obecnej pozycji wskaznika odczytu/zapisu
 * SFS_SEEK_SET - offset jest ustawiany wzgledem poczatku pliku
 * SFS_SEEK_END - offset jest ustawiany wzgledem konca pliku
 * Zwraca nowa pozycje w pliku lub kod bledu:
 * SFS_EDESC
 * SFS_BAD_OPTION
 ***********************************************************************************************************************************************/
int sfsop_lseek(int fd, int whence, int offset)
{
  int rwpointer;
  char mode;
  struct inode inod;
 
  if(get_desc(fd, &rwpointer, &mode)) 	// Odczytujemy pozycje w pliku z pliku tymczasowego
  {
    return SFS_EDESC;			// Blad pliku dostepu do pliku tymczasowego
  }
  
  if(whence == SFS_SEEK_SET)
  {
    rwpointer = offset;
  }
  else if(whence == SFS_SEEK_END)
  {
    get_inode(fd, &inod);
    rwpointer = inod.filesize + offset;
  }
  else if(whence == SFS_SEEK_CUR)
  {
    rwpointer += offset;
  }
  else
  {
    return SFS_BAD_OPTION;		// Niepoprawna opcja
  }
  
  update_desc(fd, rwpointer);
  
  return rwpointer;
}

/***********************************************************************************************************************************************
 * SFSOP CLOSE
 * Zamyka plik o deskryptorze fd
 * Zwraca:
 * SFS_ECLOSED
 * SFS_EDESC
 * SFS_OK
 ***********************************************************************************************************************************************/
int sfsop_close(int fd)
{
  // Usuń z pliku tymczasowego wpis danego procesu
  int retval = del_desc(fd);
  if(retval == -1) 			// Wystapil jakis blad
  {
    return SFS_ECLOSED;			// Plik juz zamkniety
  }
  else if(retval < 0)
  {
    return SFS_EDESC;			// Blad dostepu do pliku tymczasowego
  }

  return SFS_OK;
}

/***********************************************************************************************************************************************
 * KONIEC
 ***********************************************************************************************************************************************/
