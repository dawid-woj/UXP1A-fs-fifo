#include "sfs_vd.h"

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

int create_sfsfile(int inodes_number)
{
  return 0;
}

int open_sfsfile(void)
{
  return 0;
}

int close_sfsfile(void)
{
  return 0;
}

int reserve_inode(void)
{
  return 0;
}

void free_inode(int inodeid)
{
  return;
}

void get_inode(int inodeid, struct inode* inod)
{
  return;
}

void set_inod(int inodeid, struct inode* inod)
{
  return;
}

unsigned short reserve_block(void)
{
  return 0;
}

void free_block(unsigned short blockid)
{
  return;
}

int read_from_block(unsigned short blockid, int offset, char* buf, int nbytes)
{
  return 0;
}

int write_to_block(unsigned short blockid, int offset, char* buf, int nbytes)
{
  return 0;
}

