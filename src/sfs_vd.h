#ifndef SFS_VD_H_
#define SFS_VD_H_

#define NO_BLOCK	65535 	// Maksymalna wartosc unsigned short

struct inode 
{
  char filetype;  		// typ pliku (katalog/plik)
  char mode;      	    	// prawa dostępu	
  unsigned short nblocks;	// liczba bloków danych
  unsigned int filesize;  	// rozmiar w bajtach
  unsigned short blocks[8];	// wskaźniki na bloki*
};

int create_sfsfile(int inodes_number);

int open_sfsfile(void);

int close_sfsfile(void);

int reserve_inode(void);

void free_inode(int inodeid);

void get_inode(int inodeid, struct inode* inod);

void set_inod(int inodeid, struct inode* inod);

unsigned short reserve_block(void);

void free_block(unsigned short blockid);

int read_from_block(unsigned short blockid, int offset, char* buf, int nbytes);

int write_to_block(unsigned short blockid, int offset, char* buf, int nbytes);

#endif