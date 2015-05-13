#ifndef SFS_VD_H_
#define SFS_VD_H_
#include <stdio.h>

struct inode 
{
    char filetype;  			// typ pliku (katalog/plik)
    char mode;      	    	// prawa dostępu	
	unsigned short nblocks;		// liczba bloków danych
    unsigned int filesize;  	// rozmiar w bajtach
    unsigned short blocks[8];	// wskaźniki na bloki*
};    

struct superblock
{
	int all_blocks; 		// Liczba wszystkich bloków danych
	int free_blocks; 		// Liczba wolnych bloków danych
	int all_inodes;			// Liczba wszystkich inodów
	int free_inodes;		// Liczba wolnych inodów
	int inode_map_offset;	// Adres mapy zajętości inodów
	int block_map_offset;	// Adres mapy zajętości bloków
	int inode_table_offset; // Adres tablicy inodów
	int first_data_block;	// Adres pierwszego bloku danych
};

int reserver_inode(void);

void free_inode(int inodeid);

void get_inode(int inodeid, struct inode* inod);

void set_inod(int inodeid, struct inode* inod);

unsigned short reserver_block(void);

void free_block(unsigned short blockid);

void load_block(unsigned short blockid, char* block_data);

void store_block(unsigned short blockid, char* block_data);

#endif
