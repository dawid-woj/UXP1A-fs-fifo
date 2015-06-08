#include "simplefs_aux.h"
#include "simplefs.h"
#include "sfs_vd.h"
#include "desc_manager.h"
#include <string.h>
#include <stdio.h>

/***********************************************************************************************************************************************
 * OPEN DIR
 * Owiera katalog o podanym numerze inoda i wypelnia strukture directory
 ***********************************************************************************************************************************************/
void open_dir(int dirinodeid, struct directory* dir)
{
  struct inode dirinode;
  
  get_inode(dirinodeid, &dirinode); // Wczytanie inode
  read_from_block(dirinode.blocks[0], 0, (char*)&(dir->head), sizeof(struct dir_header)); // Wczytanie naglowka katalogu
  read_from_block(dirinode.blocks[0], sizeof(struct dir_header), (char*)dir->entries, sizeof(struct dir_entry) * dir->head.entries); // Wczytanie wpisow katalogu
  dir->data_block = dirinode.blocks[0];
  dir->found_object_position = SFS_NOT_FOUND;
}

/***********************************************************************************************************************************************
 * CLOSE DIR
 * Zamyka katalog zapisujac jego zawartosc zgodnie ze struktura directory
 ***********************************************************************************************************************************************/
void close_dir(struct directory* dir)
{
  write_to_block(dir->data_block, 0, (char*)&(dir->head), sizeof(struct dir_header)); // Zapis naglowka katalogu
  write_to_block(dir->data_block, sizeof(struct dir_header), (char*)dir->entries, sizeof(struct dir_entry) * dir->head.entries); // Zapis wpisow katalogu
}

/***********************************************************************************************************************************************
 * ADD TO DIR
 * Dodaje nowy wpis katalogu
 * Zwraca:
 * SFS_OK - sukces,
 * SFS_DIR_FULL - brak miejsca w katalogu
 ***********************************************************************************************************************************************/
int add_to_dir(int inodeid, char type, char* filename, struct directory* dir)
{
  if(dir->head.entries < MAX_DIR_ENTRIES)
  {
    dir->entries[dir->head.entries].inode_number = inodeid;
    dir->entries[dir->head.entries].type = type;
    strcpy(dir->entries[dir->head.entries].filename, filename);
    ++(dir->head.entries);
    return SFS_OK;
  }
  return SFS_DIR_FULL;
}

/***********************************************************************************************************************************************
 * DEL FROM DIR
 * Usuwa z katalogu wpis wskazany przez dir->found_object_position
 ***********************************************************************************************************************************************/
void del_from_dir(struct directory* dir)
{
  dir->entries[dir->found_object_position].inode_number = dir->entries[dir->head.entries - 1].inode_number;
  dir->entries[dir->found_object_position].type = dir->entries[dir->head.entries - 1].type;
  strcpy(dir->entries[dir->found_object_position].filename, dir->entries[dir->head.entries - 1].filename);
  --(dir->head.entries);
}

/***********************************************************************************************************************************************
 * FIND IN DIRECTORY
 * Poszukuje obiektu o nazwie name i typie type w katalogu o nr inoda dirinodeid
 * W strukturze dir zapisuje zawartosc przeszukiwanego katalogu, ustawiajac pole found_object_position na indeks znalezionego obiektu
 * Zwraca nr inoda znalezionego obiektu lub odpowiedni kod bledu (SFS_BAD_NAME - pusta nazwa, SFS_NOT_FOUND - nie znaleiono)
 ***********************************************************************************************************************************************/
int find_in_directory(int dirinodeid, char* name, char type, struct directory* dir)
{
  int i;
  
  if(*name == 0) // Pusta nazwa obiektu
  {
    return SFS_BAD_NAME;
  }
  open_dir(dirinodeid, dir); // Wcztanie zawartosci katalogu
  for(i = 0; i < dir->head.entries; ++i)
  {
    if((strcmp(name, dir->entries[i].filename) == 0) && (dir->entries[i].type & type))
    {
      dir->found_object_position = i;
      return dir->entries[i].inode_number;
    }
  }
  return SFS_NOT_FOUND;
}

/***********************************************************************************************************************************************
 * LOOKUP
 * Poszukuje obiektu wskazywanego przez sciezke path o typie type
 * W strukturze dir zapisuje zawartosc katalogu zawierajacego szukany obiekt, ustawiajac pole found_object_position na indeks znalezionego obiektu
 * Wartosc wskazywana przez namepointer zostaje ustawiona tak aby wskazywala koncowy obiekt sciezki
 * Zwraca nr inoda znalezionego obiektu lub odpowiedni kod bledu:
 * SFS_NAME_TO_LONG - w sciezke wystepuje zbyt dluga nazwa pliku/katalogu
 * SFS BAD_PATH - niepoprawna sciezka
 * SFS_NOT_FOUND - nie znaleiono obiektu koncowego
 * SFS_BAD_NAME - pusta nazwa koncowego obiektu
 ***********************************************************************************************************************************************/
int lookup(char* path, char** namepointer, int type, struct directory* dir)
{
  char name[MAX_NAME_LENGTH + 1];
  char* pathpointer = path;
  int i;
  int dirnodeid = SFS_ROOT_INODE;
  int status = SEARCHING;
  
  dir->head.own_inode = SFS_ROOT_INODE;
  dir->head.parent_dir = SFS_ROOT_INODE;
  
  while(status == SEARCHING)
  { 
    *namepointer = pathpointer;
    // Pobranie tokena z path i zapisanie go do name
    for(i = 0; i <= MAX_NAME_LENGTH; ++i)
    {
      if(*pathpointer == 0)
      {
	// Koniec sciezki
	status = PATHS_END;
	break;
      }
      else if(*pathpointer == '/')
      {
	// Koniec tokena
	++pathpointer;
	status = DELIMITER;
	break;
      }
      name[i] = *(pathpointer++); // Przepisz znak do bufora pomocniczego
    }

    if((i > MAX_NAME_LENGTH) && (status == SEARCHING)) // Bufor zapelniony, ale nie znaleziono ani '/' ani konca napisu
    {
      return SFS_NAME_TO_LONG;
    }
    name[i] = 0;
    if(strcmp(name, "..") == 0)
    {
      dirnodeid = dir->head.parent_dir;
    }
    else if(strcmp(name, ".") == 0)
    {
      dirnodeid = dir->head.own_inode;
    }
    else if(status == PATHS_END) // Jestesmy w finalnym katalogu - poszukujemy obiektu koncowego
    {
      return find_in_directory(dirnodeid, name, type, dir);
    }
    else // status == DELIMITER - Przechodzimy przez kolejny katalog
    {
      dirnodeid = find_in_directory(dirnodeid, name, SFS_DIRECTORY, dir);
      if(dirnodeid == SFS_NOT_FOUND)
      {
	break;
      }
    }
    status = SEARCHING;
  }
  return SFS_BAD_PATH;
}

/***********************************************************************************************************************************************
 * DELETE FILE
 * Zwalnia bloki danych i inode pliku o nr inoda fileinodeid
 ***********************************************************************************************************************************************/
void delete_file(int fileinodeid)
{
  struct inode fileinode;
  int i;
  
  get_inode(fileinodeid, &fileinode);
  // Petla zwalniajaca bloku adresowane bezposrednio
  for(i = 0; i < SFS_DIRECT_BLOCKS; ++i)
  {
    if(fileinode.nblocks == 0)
    {
      break;
    }  
    free_block(fileinode.blocks[i]);
    --fileinode.nblocks;
  }
  if(fileinode.nblocks > 0) // Sa jeszcze bloki adresowane posrednio
  {
    unsigned short blocks[fileinode.nblocks];
    read_from_block(fileinode.blocks[SFS_INDIRECT_POINTER], 0, (char*)blocks, sizeof(unsigned short) * fileinode.nblocks); // Wczytanie adresow blokow
    // Petla zwalniajaca bloki adresowane posrednio
    for(i = 0; i < fileinode.nblocks; ++i)
    {
      free_block(blocks[i]);
    }
    free_block(fileinode.blocks[SFS_INDIRECT_POINTER]); // Zwolnienie bloku adresowania posredniego
  }
  // Zwolnienie inoda pliku
  free_inode(fileinodeid);
  // Usuniecie pliku tymczasowego zwiazengo z danym plikiem
  del_descfile(fileinodeid);
}

/***********************************************************************************************************************************************
 * DELETE DIRECTORY
 * Zwalnia blok danych i inode katalogu o nr inoda dirinodeid
 * Usuwa rekurencyjnie zawartosc katalogu
 ***********************************************************************************************************************************************/
void delete_directory(int dirinodeid)
{
  struct directory dir;
  int i;
  
  open_dir(dirinodeid, &dir); // Wczytanie zawartosci katalogu
  for(i = 0; i < dir.head.entries; ++i) // Usuwanie zawartosci katalogu
  {
    if(dir.entries[i].type == SFS_FILE)
    {
      delete_file(dir.entries[i].inode_number);
    }
    else
    {
      delete_directory(dir.entries[i].inode_number);
    }
  }
  free_block(dir.data_block); // Zwolnienie bloku danych katalogu
  free_inode(dir.head.own_inode); // Zwolnienie wlasnego inoda
}

/***************************************************************************
 * Funkcje diagnostyczne
 **************************************************************************/

/***********************************************************************************************************************************************
 * DEBUG LIST DIR
 * Wypisuje zawartosc podanego katalogu na stdout
 ***********************************************************************************************************************************************/

void debug__list_dir(char* path)
{
  int dirnodeid, i;
  char* garbage;
  struct directory dir;
  
  open_sfsfile();
  
  printf("Dirinfo: %s\n", path);
  
  if(*path == 0) // Odwolanie do katalogu root
  {
    dirnodeid = SFS_ROOT_INODE;
  }
  else
  {
    dirnodeid = lookup(path, &garbage, SFS_DIRECTORY, &dir);
  }
  
  if(dirnodeid < 0)
  {
    printf("Directory not found\n");
    return;
  }
  
  open_dir(dirnodeid, &dir);
  
  printf("#\tnode\ttype\tname\n");
  printf("-\t%d\t%hhX\t.\n", dir.head.own_inode, SFS_DIRECTORY);
  printf("-\t%d\t%hhX\t..\n", dir.head.parent_dir, SFS_DIRECTORY);
  
  for(i = 0; i < dir.head.entries; ++i)
  {
    printf("%d\t%d\t%hhX\t%s\n", i, dir.entries[i].inode_number, dir.entries[i].type, dir.entries[i].filename);
  }
  
  close_sfsfile();
  
}



























