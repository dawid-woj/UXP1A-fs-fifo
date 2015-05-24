#include "sfs_vd.h"
#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define		MAX_BLOCKS		65536	// Maksymalna liczba blokow danych (2^16)
#define		INODES_NUMBER		512	// Liczba inodow SFS


/***************************************************************************
 * Zmienne globalne modulu
 **************************************************************************/

int sfsfd;			// Deskryptor pliku SFS
struct superblock sblock;	// Kopia superbloku w pamieci
char* inode_map;		// Kopia mapy inodow w pamieci
char* block_map;		// Kopia mapy blokow w pamieci

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

struct superblock
{
  int all_blocks; 		// Liczba wszystkich bloków danych
  int free_blocks; 		// Liczba wolnych bloków danych
  int all_inodes;		// Liczba wszystkich inodów
  int free_inodes;		// Liczba wolnych inodów
  int inode_map_offset;		// Adres mapy zajętości inodów
  int block_map_offset;		// Adres mapy zajętości bloków
  int inode_table_offset; 	// Adres tablicy inodów
  int first_data_block;		// Adres pierwszego bloku danych
};

/***************************************************************************
 * API modulu
 **************************************************************************/

/* Tworzy nowy plik SFS oraz rezerwuje inode i blok danych na katalog root
 * Zwraca:
 * SFSVD_OK - sukces
 * SFSVD_EOPEN - niepowodzenie przy tworzeniu pliku SFS
 * SFSVD_RESIZE - niepowodzenie przy ustalaniu rozmiaru pliku SFS
 */
int create_sfsfile(char* name, int blocks_per_inode)
{
  struct inode rootnode;
  unsigned char first_mask = 0x80; // 1000 000 (bitowo)
  
  // Przygotuj superblok
  sblock.all_blocks = INODES_NUMBER * blocks_per_inode;
  if (sblock.all_blocks > MAX_BLOCKS)
  {
    sblock.all_blocks = MAX_BLOCKS; // Przekroczono dopuszczalna przez adresowanie typem unsigned short liczbe blokow -> korekcja wartosci
  }
  sblock.free_blocks = sblock.all_blocks - 1; // Zaraz zajmiemy 1 blok na katalog root
  sblock.all_inodes = INODES_NUMBER;
  sblock.free_inodes = INODES_NUMBER - 1; // Rezerwujemy 1 inode na katalog root
  sblock.block_map_offset = sizeof(sblock); // Mapa blokow zaraz za superblokiem
  sblock.inode_map_offset = sblock.block_map_offset + sblock.all_blocks / 8; // Wyznaczamy poczatek mapy inodow (mapa inodow zaraz po mapie blokow)
  sblock.inode_table_offset = sblock.inode_map_offset + INODES_NUMBER / 8; // Wyznaczamy poczatek tablicy inodow (tablica inodow zaraz po mapie inodow)
  sblock.first_data_block = sblock.inode_table_offset + INODES_NUMBER * sizeof(rootnode); // Wyznaczamy poczatek blokow danych (bloki danych zaraz po tablicy inodow)
  
  // Otworzenie pliku SFS
  sfsfd = open(name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR + S_IWUSR + S_IRGRP + S_IWGRP);
  if (sfsfd < 0)
  {
    return SFSVD_EOPEN;
  }
  
  // Ustalenie rozmiaru pliku SFS -> plik powinien zostac wypelniony zerami
  if (ftruncate(sfsfd, sblock.first_data_block + sblock.all_blocks * SFS_BLOCK_SIZE) < 0)
  {
    return SFSVD_ERESIZE;
  }
  
  // Zajecie bloku roota (pierwszy wolny blok)
  lseek(sfsfd, sblock.block_map_offset, SEEK_SET);
  write(sfsfd, &first_mask, 1);
  
  // Zajecie inodu roota (pierwszy wolny inode)
  lseek(sfsfd, sblock.inode_map_offset, SEEK_SET);
  write(sfsfd, &first_mask, 1);
  
  // Zapisanie inoda roota
  rootnode.filetype = SFS_DIRECTORY;
  rootnode.mode = SFS_RDWR;
  rootnode.nblocks = 1;
  rootnode.filesize = SFS_BLOCK_SIZE; // Rozmiar katalogu to jeden blok (???)
  rootnode.blocks[0] = SFS_ROOT_BLOCK;
  lseek(sfsfd, sblock.inode_table_offset, SEEK_SET);
  write(sfsfd, (char*) &rootnode, sizeof(rootnode));
  
  return SFSVD_OK;
}

/* Otwarcie pliku SFS, odczyt superbloku oraz map inodow i blokow
 * Zwraca:
 * SFSVD_OK - sukces
 * SFSVD_EOPEN - otwarcie pliku SFS zakonczone niepowodzeniem
 */
int open_sfsfile(void)
{
  sfsfd = open(SFS_LINK_NAME, O_RDWR);
  if (sfsfd < 0)
  {
    return SFSVD_EOPEN;
  }
  
  // Odczyt superbloku
  read(sfsfd, (char*) &sblock, sizeof(sblock));
  
  // Odczyt mapy blokow
  block_map = malloc(sblock.all_blocks / 8);
  read(sfsfd, block_map, sblock.all_blocks / 8);
  
  // Odczyt mapy inodow
  inode_map = malloc(sblock.all_inodes / 8);
  read(sfsfd, inode_map, sblock.all_inodes / 8);

  return SFSVD_OK;
}

/* Zapisanie superblok, map inodow i blokow oraz zamkniecie pliku SFS 
 * Zwraca:
 * SFSVD_OK - sukces
 */
int close_sfsfile(void)
{
  // Zapis superbloku
  lseek(sfsfd, 0, SEEK_SET);
  write(sfsfd, (char*) &sblock, sizeof(sblock));
  
  // Zapis mapy blokow
  write(sfsfd, block_map, sblock.all_blocks / 8);
  free(block_map);
  
  // Zapis mapy inodow
  write(sfsfd, inode_map, sblock.all_inodes / 8);
  free(inode_map);
  
  close(sfsfd); // Zamkniecie pliku SFS
  return SFSVD_OK;
}

/* Zajcie inoda - funkcja odnajduje pierwszy wolny inode na mapie inodow, oznacza go jako zajety i aktualizuje superblok
 * Zwaraca:
 * Indeks zarezerwowanego inoda lub SFS_NO_INODE w przypadku braku wolnych inodow
 */
int reserve_inode(void)
{
  int i, j;
  int inode_map_size = sblock.all_inodes / 8;
  
  if(sblock.free_inodes) // Czy sa wolne inody
  {
    for(i  = 0; i < inode_map_size; ++i) // Petla po bajtach bitmapy
    {
      if(inode_map[i] != 0xFF)
      {
	unsigned char mask = 0x80;
	for(j = 0; j < 8; ++j) // Petla po bitach bitmapy
	{
	  if((inode_map[i] & mask) == 0)
	  {
	    // Znaleziono wolny inode
	    inode_map[i] |= mask; // Zaznacz znaleziony inode na mapie bitowej
	    --sblock.free_inodes; // Odnotuj fakt w superbloku
	    return (8*i + j); // Zwroc indeks znalezionego inoda 
	  }
	  mask = mask >> 1; // Przesun maske w prawo
	}
      }
    }
  }
  return SFS_NO_INODE; // Brak wolnych inodow
}

/* Zwolnienie inoda o podanym indeksie
 * Funkcja zaznacza fakt zwolnienia inoda na mapie inodow i w superbloku
 */
void free_inode(int inodeid)
{
  unsigned char mask = 0x80;
  
  mask = ~(mask >> (inodeid % 8)); // Przygotowanie maski
  inode_map[inodeid/8] &= mask; // Wygaszenie odpowiedniego bitu mapy
  ++sblock.free_inodes; // Odnotuj fakt w superbloku
}

/* Odczyt zawartosci inoda
 * Funkcja wypelnia strukture wskazywana przez inod zawartoscia inoda o indeksie inodeid
 */
void get_inode(int inodeid, struct inode* inod)
{
  lseek(sfsfd, sblock.inode_table_offset + inodeid*sizeof(*inod), SEEK_SET);
  read(sfsfd, (char*) inod, sizeof(*inod));
}

/* Zapis zawartosci inoda
 * Funkcja nadpisuje zawartosc inoda o indeksie inodeid zawartoscia struktury wskazywanej przez inod
 */
void set_inode(int inodeid, struct inode* inod)
{
  lseek(sfsfd, sblock.inode_table_offset + inodeid*sizeof(*inod), SEEK_SET);
  write(sfsfd, (char*) inod, sizeof(*inod));
  return;
}

/* Zajcie bloku danych
 * Funkcja odnajduje pierwszy wolny blok na mapie blokow, oznacza go jako zajety i aktualizuje superblok
 * Zwaraca:
 * Indeks zarezerwowanego bloku lub SFS_NO_BLOCK w przypadku braku wolnych blokow
 */
unsigned short reserve_block(void)
{
  int i, j;
  int block_map_size = sblock.all_blocks / 8;
  
  if(sblock.free_blocks) // Czy sa wolne inody
  {
    for(i  = 0; i < block_map_size; ++i) // Petla po bajtach bitmapy
    {
      if(block_map[i] != 0xFF)
      {
	unsigned char mask = 0x80;
	for(j = 0; j < 8; ++j) // Petla po bitach bitmapy
	{
	  if((block_map[i] & mask) == 0)
	  {
	    // Znaleziono wolny blok
	    block_map[i] |= mask; // Zaznacz znaleziony blok na mapie bitowej
	    --sblock.free_blocks; // Odnotuj fakt w superbloku
	    return (unsigned short)(8*i + j); // Zwroc indeks znalezionego bloku
	  }
	  mask = mask >> 1; // Przesun maske w prawo
	}
      }
    }
  }
  return SFS_NO_BLOCK; // Brak wolnych inodow
}

/* Zwolnienie bloku danych o podanym indeksie
 * Funkcja zaznacza fakt zwolnienia bloku na mapie blokow i w superbloku
 */
void free_block(unsigned short blockid)
{
  unsigned char mask = 0x80;
  
  mask = ~(mask >> (blockid % 8)); // Przygotowanie maski
  block_map[blockid/8] &= mask; // Wygaszenie odpowiedniego bitu mapy
  ++sblock.free_blocks; // Odnotuj fakt w superbloku
}

/* Odczyt z bloku danych
 * Funkcja odczytuje dane z bloku, jednak nie wiecej niz wynika to z zakresu bloku (SFS_BLOCK_SIZE)
 * Jesli offset + nbytes > SFS_BLOCK_SIZE, to zostana odczytane tylko dane od miejsca offset do konca bloku
 * Parametry:
 * blocki - identyfikator bloku
 * offset - przesuniecie punktu poczatku odczytu wewnatrz bloku (w bajtach)
 * buf - bufor odczytu
 * nbytes - liczba bajtow do odczytania
 * Zwraca:
 * liczba odczytanych bajtow
 */
int read_from_block(unsigned short blockid, int offset, char* buf, int nbytes)
{
  int len = (offset + nbytes > SFS_BLOCK_SIZE) ? (SFS_BLOCK_SIZE - offset) : (nbytes); // Przelicz liczbe odcztywanych bajtow
  lseek(sfsfd, sblock.first_data_block + blockid * SFS_BLOCK_SIZE + offset, SEEK_SET); // Ustaw wskaznik odczytu
  return (read(sfsfd, buf, len)); 
}

/* Zapis do bloku danych
 * Funkcja zapisuje dane do bloku, jednak nie wiecej niz wynika to z zakresu bloku (SFS_BLOCK_SIZE)
 * Jesli offset + nbytes > SFS_BLOCK_SIZE, to zostana zapisane tylko dane od miejsca offset do konca bloku
 * Parametry:
 * blocki - identyfikator bloku
 * offset - przesuniecie punktu poczatku zapisu wewnatrz bloku (w bajtach)
 * buf - bufor zapisu
 * nbytes - liczba bajtow do zapisania
 * Zwraca:
 * liczba zapisanych bajtow
 */
int write_to_block(unsigned short blockid, int offset, char* buf, int nbytes)
{
  int len = (offset + nbytes > SFS_BLOCK_SIZE) ? (SFS_BLOCK_SIZE - offset) : (nbytes); // Przelicz liczbe zapisywanych bajtow
  lseek(sfsfd, sblock.first_data_block + blockid * SFS_BLOCK_SIZE + offset, SEEK_SET); // Ustaw wskaznik zapisu
  return (write(sfsfd, buf, len)); 
}

/***************************************************************************
 * Funkcje diagnostyczne
 **************************************************************************/

/* Funkcja testowa
 * Drukuje zawartosci superbloku i map blokow/inodow na stdout (mapy w postaci hex)
 */
void debug__analyse_sfsfile(char* filename)
{
  int i;
  char c;
  
  sfsfd = open(filename, O_RDONLY);
  read(sfsfd, (char*) &sblock, sizeof(sblock));
  printf("All blocks       = %d\n", sblock.all_blocks);
  printf("Free blocks      = %d\n", sblock.free_blocks);
  printf("All inodes       = %d\n", sblock.all_inodes);
  printf("Free inodes      = %d\n", sblock.free_inodes);
  printf("Block map size   = %d\n", sblock.inode_map_offset - sblock.block_map_offset);
  printf("Inode map size   = %d\n", sblock.inode_table_offset - sblock.inode_map_offset);
  printf("Inode table size = %d\n", sblock.first_data_block - sblock.inode_table_offset);
  printf("Block map:\n");
  lseek(sfsfd, sblock.block_map_offset, SEEK_SET);
  for(i = 0; i < sblock.inode_map_offset - sblock.block_map_offset; ++i)
  {
    read(sfsfd, &c, 1);
    printf("%hhX\t", c);
    if(i%8 == 7) printf("\n");
  }
  printf("Inode map:\n");
  lseek(sfsfd, sblock.inode_map_offset, SEEK_SET);
  for(i = 0; i < sblock.inode_table_offset - sblock.inode_map_offset; ++i)
  {
    read(sfsfd, &c, 1);
    printf("%hhX\t", c);
    if(i%8 == 7) printf("\n");
  }
  close(sfsfd);
}

/* Funkcja testowa
 * Drukuje zawartosc inoda na stdout
 */
void debug__print_inode(struct inode* inod)
{
  int i;
  printf("Inodeinfo\n");
  printf("type:           %hhX\n", inod->filetype);
  printf("mode:           %hhX\n", inod->mode);
  printf("nblocks:        %hu\n", inod->nblocks);
  printf("filesize:       %u\n", inod->filesize);
  
  for(i = 0; i < 8; ++i)
  {
    printf("datablock id: %hu\n", inod->blocks[i]);
  }
}

/* Funkcja testowa
 * Drukuje zawartosc bloku danych na stdout (w postaci znakowej)
 * Paramter range okresla liczbe bajtow do wypisania
 */
void debug__print_block(unsigned short blockid, int range)
{
  int i;
  char data[SFS_BLOCK_SIZE];
  
  read_from_block(blockid, 0, data, SFS_BLOCK_SIZE);
  range = (range > SFS_BLOCK_SIZE) ? SFS_BLOCK_SIZE : range;
  
  printf("Blockinfo\n");
  for(i = 0; i < range; ++i)
  {
    if (data[i] & 0xF0) 
      printf("%hhX ", data[i]);
    else
      printf("0%hhX ", data[i]);
      //printf("- ");
    if(i%32 == 31) printf("\n");
    else if(i%8 == 7) printf(" ");
  }
}
