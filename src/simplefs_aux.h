#ifndef SIMPLEFS_AUX_H_
#define SIMPLEFS_AUX_H_

#define MAX_DIR_ENTRIES	127	// Masksymalna liczba wspisow katalogu (nie liczac naglowka katalogu)
#define MAX_NAME_LENGTH 26	// Maksymalna dlugosc nazwy pliku/katalogu
#define FOUND		0
#define SEARCHING	1
#define DELIMITER	2
#define	PATHS_END	3

/***************************************************************************
 * Struktury danych
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

/* Struktura agregujaca informacje o katalogu */
struct directory 
{
  struct dir_header head;
  struct dir_entry entries[MAX_DIR_ENTRIES];
  unsigned short data_block;
  int found_object_position;
};

/***************************************************************************
 * Funkcje API
 ***************************************************************************/

/***********************************************************************************************************************************************
 * OPEN DIR
 * Owiera katalog o podanym numerze inoda i wypelnia strukture directory
 ***********************************************************************************************************************************************/
void open_dir(int dirinodeid, struct directory* dir);

/***********************************************************************************************************************************************
 * CLOSE DIR
 * Zamyka katalog zapisujac jego zawartosc zgodnie ze struktura directory
 ***********************************************************************************************************************************************/
void close_dir(struct directory* dir);

/***********************************************************************************************************************************************
 * ADD TO DIR
 * Dodaje nowy wpis katalogu
 * Zwraca:
 * SFS_OK - sukces,
 * SFS_DIR_FULL - brak miejsca w katalogu
 ***********************************************************************************************************************************************/
int add_to_dir(int inodeid, char type, char* filename, struct directory* dir);

/***********************************************************************************************************************************************
 * DEL FROM DIR
 * Usuwa z katalogu wpis wskazany przez dir->found_object_position
 ***********************************************************************************************************************************************/
void del_from_dir(struct directory* dir);

/***********************************************************************************************************************************************
 * FIND IN DIRECTORY
 * Poszukuje obiektu o nazwie name i typie type w katalogu o nr inoda dirinodeid
 * W strukturze dir zapisuje zawartosc przeszukiwanego katalogu, ustawiajac pole found_object_position na indeks znalezionego obiektu
 * Zwraca nr inoda znalezionego obiektu lub odpowiedni kod bledu (SFS_BAD_NAME - pusta nazwa, SFS_NOT_FOUND - nie znaleiono)
 ***********************************************************************************************************************************************/
int find_in_directory(int dirinodeid, char* name, char type, struct directory* dir);

/***********************************************************************************************************************************************
 * GET OBJECTS INODE AND DIRECTORY
 * Poszukuje obiektu wskazywanego przez sciezke path o typie type
 * W strukturze dir zapisuje zawartosc katalogu zawierajacego szukany obiekt, ustawiajac pole found_object_position na indeks znalezionego obiektu
 * Wartosc wskazywana przez namepointer zostaje ustawiona tak aby wskazywala koncowy obiekt sciezki
 * Zwraca nr inoda znalezionego obiektu lub odpowiedni kod bledu:
 * SFS_NAME_TO_LONG - w sciezke wystepuje zbyt dluga nazwa pliku/katalogu
 * SFS BAD_PATH - niepoprawna sciezka
 * SFS_NOT_FOUND - nie znaleiono obiektu koncowego
 * SFS_BAD_NAME - pusta nazwa koncowego obiektu
 ***********************************************************************************************************************************************/
int get_objects_inode_and_directory(char* path, char** namepointer, int type, struct directory* dir);

/***********************************************************************************************************************************************
 * DELETE FILE
 * Zwalnia bloki danych i inode pliku o nr inoda fileinodeid
 ***********************************************************************************************************************************************/
void delete_file(int fileinodeid);

/***********************************************************************************************************************************************
 * DELETE DIRECTORY
 * Zwalnia blok danych i inode katalogu o nr inoda dirinodeid
 * Usuwa rekurencyjnie zawartosc katalogu
 ***********************************************************************************************************************************************/
void delete_directory(int dirinodeid);

#endif
