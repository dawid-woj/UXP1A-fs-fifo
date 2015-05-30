#ifndef SFS_VD_H_
#define SFS_VD_H_

#define SFS_LINK_NAME		"/tmp/sfsfile"	// Nazwa twardego dowiazania do pliku SFS
#define	SFS_BLOCK_SIZE		4096	// Rozmiar bloku danych w bajtach
#define SFS_FILE		1	// Plik zwykly
#define SFS_DIRECTORY		2	// Katalog
#define SFS_NO_BLOCK		0 	// Wartosc sygnalizujaca brak wolnych blokow
#define SFS_NO_INODE		-1	// Wartosc sygnalizujaca brak wolnych inodow
#define	SFS_ROOT_INODE		0	// Nr inoda katalogu root
#define	SFS_ROOT_BLOCK		0	// Nr bloku katalogu root
#define SFS_DIRECT_BLOCKS	7	// Liczba blokow danych adresowanych bezposrednio
#define SFS_INDIRECT_BLOCKS	2048	// Liczba blokow danych adresowanych posrednio
#define SFS_INDIRECT_POINTER	7	// Indeks wskaznika do blokow adresowanych bezposrednio

// Kody bledow:
#define SFSVD_EOPEN	-1	// Blad otwarcia pliku SFS
#define SFSVD_ERESIZE	-2	// Blad przy zmianie rozmiaru pliku SFS

// Kod sukcesu
#define SFSVD_OK	0	// Kod sukcesu wykonania funkcji

/* Struktura opisujaca inode */
struct inode 
{
  char filetype;  		// typ pliku (katalog/plik)
  char mode;      	    	// prawa dostępu	
  unsigned short nblocks;	// liczba bloków danych
  unsigned int filesize;  	// rozmiar w bajtach
  unsigned short blocks[8];	// wskaźniki na bloki*
};

/*********************************************************************************************************************
 * UWAGA ODNOSNIE KORZYSTANIA Z MODULU
 * Aby korzystac z funkcjonalnosci modulu nalezy najpierw otworzyc plik sfs za pomoca open_sfsfile()/create_sfsfile(),
 * co zapewnia odswiezenie kopii superbloku i map bitowych w pamieci programu,
 * a po zakonczeniu pracy wywolac close_sfsfile() w celu zatwierdzenia zmian w superbloku i mapach bitowych.
 * Niedotyczny to funkcji testowych, ktore robia to we wlasnym zakresie.
 * Generalnie funkcje modulu nie sprawdzaja poprawnosci podawanych im parametrow.
 *********************************************************************************************************************/

//----------------------------- API MODULU -----------------------------//

/* Tworzy nowy plik SFS oraz rezerwuje inode i blok danych na katalog root
 * Zwraca:
 * SFSVD_OK - sukces
 * SFSVD_EOPEN - niepowodzenie przy tworzeniu pliku SFS
 * SFSVD_RESIZE - niepowodzenie przy ustalaniu rozmiaru pliku SFS
 */
int create_sfsfile(char* name, int blocks_per_inode);

/* Otwarcie pliku SFS, odczyt superbloku oraz map inodow i blokow
 * Zwraca:
 * SFSVD_OK - sukces
 * SFSVD_EOPEN - otwarcie pliku SFS zakonczone niepowodzeniem
 */
int open_sfsfile(void);

/* Zapisanie superblok, map inodow i blokow oraz zamkniecie pliku SFS 
 * Zwraca:
 * SFSVD_OK - sukces
 */
int close_sfsfile(void);

/* Zajcie inoda - funkcja odnajduje pierwszy wolny inode na mapie inodow, oznacza go jako zajety i aktualizuje superblok
 * Zwaraca:
 * Indeks zarezerwowanego inoda lub SFS_NO_INODE w przypadku braku wolnych inodow
 */
int reserve_inode(void);

/* Zwolnienie inoda o podanym indeksie
 * Funkcja zaznacza fakt zwolnienia inoda na mapie inodow i w superbloku
 */
void free_inode(int inodeid);

/* Odczyt zawartosci inoda
 * Funkcja wypelnia strukture wskazywana przez inod zawartoscia inoda o indeksie inodeid
 */
void get_inode(int inodeid, struct inode* inod);

/* Zapis zawartosci inoda
 * Funkcja nadpisuje zawartosc inoda o indeksie inodeid zawartoscia struktury wskazywanej przez inod
 */
void set_inode(int inodeid, struct inode* inod);

/* Zajcie bloku danych
 * Funkcja odnajduje pierwszy wolny blok na mapie blokow, oznacza go jako zajety i aktualizuje superblok
 * Zwraca:
 * Indeks zarezerwowanego bloku lub SFS_NO_BLOCK w przypadku braku wolnych blokow
 */
unsigned short reserve_block(void);

/* Zwolnienie bloku danych o podanym indeksie
 * Funkcja zaznacza fakt zwolnienia bloku na mapie blokow i w superbloku
 */
void free_block(unsigned short blockid);

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
int read_from_block(unsigned short blockid, int offset, char* buf, int nbytes);

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
int write_to_block(unsigned short blockid, int offset, char* buf, int nbytes);


//----------------------------- FUNKCJE DIAGNOSTYCZNE -----------------------------//

/* Funkcja testowa
 * Drukuje zawartosci superbloku i map blokow/inodow na stdout (mapy w postaci hex)
 */
void debug__analyse_sfsfile(char* filename);

/* Funkcja testowa
 * Drukuje zawartosc inoda o nr inodeid na stdout
 */
void debug__print_used_inodes(void);

/* Funkcja testowa
 * Drukuje zawartosc bloku danych na stdout (w postaci hex)
 * Paramter range okresla liczbe bajtow do wypisania
 */
void debug__print_block(unsigned short blockid, int range);

#endif