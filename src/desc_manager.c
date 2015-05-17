#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "desc_manager.h"

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

struct fd_entry
{
  int owner_pid;	// Pid właściciela deskryptora
  int rwoffset;		// Pozycja wskaźnika zapisu/odczytu
  char mode;		// Tryb w którym otworzono plik
};

/***************************************************************************
 * Wewnetrzne funkcje
 **************************************************************************/

/*
Tworzy z sciezke 'TMP_DIR/fd.tmp'.
'filepath' - tablica na finalna sciezke (musi byc odpowiednio duza)
'fd' - deskryptor pliku (jako nazwa pliku)
'temporary' - flaga mowiaca czy tworzymy plik na czas przepisywania zawartosci 'fd.tmp'
Zwraca: utworzony lancuch znakowy: filepath - sukces, NULL - blad
*/
char * createFilePath(char * filepath, int fd, char temporary)
{
	char * ptr = filepath; 
	int c;
	if ((c = sprintf(ptr, "%s", TMP_DIR)) < 0)
		return NULL;
	ptr += c;
	if ((c = sprintf(ptr, "%d", fd)) < 0)
		return NULL;
	ptr += c;
	if (temporary)
	{
		if (sprintf(ptr, "%s", ".ttmp") < 0)
			return NULL;
	}
	else
	{
		if (sprintf(ptr, "%s", ".tmp") < 0)
			return NULL;
	}
	return filepath;
}

/***************************************************************************
 * API modulu
 **************************************************************************/

int add_desc(int fd, char mode)
{
	// stworz nazwe "fd.tmp":
	char filename[16];
	if (createFilePath(filename, fd, 0) == NULL)
		return -1;

	// otworz i/lub utworz plik fd.tmp:
	int filed = open(filename, O_CREAT|O_WRONLY|O_APPEND,
		S_IRUSR+S_IWUSR+S_IRGRP+S_IWGRP+S_IROTH+S_IWOTH); // tworzy plik, jesli nie istnieje; zapis na koniec pliku
	if (filed < 0)
		return -2;
	
	// stworz wpis i wpisz na koniec pliku fd.tmp:
	struct fd_entry newEntry;
	newEntry.owner_pid = getpid();;
	newEntry.rwoffset = 0;
	newEntry.mode = mode;
	if (write(filed, &newEntry, sizeof(newEntry)) < 1)
		return -3;
	close(filed);

	return 0;
}

int del_desc(int fd)
{
	// stworz nazwe "fd.tmp":
	char filename[16];
	if (createFilePath(filename, fd, 0) == NULL)
		return -1;

	// otworz plik fd.tmp:
	int filed = open(filename, O_RDONLY);	// read binary: czytanie z pliku, plik musi istniec
	if (filed < 0)
		return -2;

	// utworz plik tymczasowy, do ktorego bedziemy przepisywac wpisy:
	char tmpname[16];
	if (createFilePath(tmpname, fd, 1) == NULL)
		return -1;
	int tmpfiled = open(tmpname, O_CREAT|O_TRUNC|O_WRONLY|O_APPEND,
		S_IRUSR+S_IWUSR+S_IRGRP+S_IWGRP+S_IROTH+S_IWOTH);	// tworzy plik, jesli nie istnieje; zapis na koniec pliku
	if (tmpfiled < 0)
		return -3;

	// ustalamy czy plik 'fd.tmp' zawiera tylko 1 wpis:
	char oneEntry = 0;
	off_t pos = lseek(filed, 0, SEEK_END);
	if (pos < 0)
	{
		close(filed);
		close(tmpfiled);
		return -1;
	}
	else if (pos == sizeof(struct fd_entry))
		oneEntry = 1;
	// przepisz zawartosc pliku 'fd.tmp' bez usuwanego wpisu do pliku tymczasowego:
	lseek(filed, 0, SEEK_SET);
	while (1)
	{
		// odczytaj deskryptor:
		struct fd_entry entry;
		int readBytes = read(filed, &entry, sizeof(entry));
		// blad czy end-of-file?
		if (readBytes == 0)			// eof
			break;
		else if (readBytes < sizeof(entry))	// blad
		{
			close(filed);
			return -3;
		}
		// jesli wpis do usuniecia jest jedynym => usun plik 'fd.tmp':
		if (entry.owner_pid == getpid() && oneEntry)
		{
			// zamykamy i usuwamy plik tymczasowy:
			close(tmpfiled);
			if (remove(tmpname) != 0)
				return -1;
			// zamykamy i usuwamy plik 'fd'tmp':
			close(filed);
			if (remove(filename) != 0)
				return -1;
			return 0;
		}
		// jesli to ten do usuniecia -> pomin zapis, w p.p. zapisz do pliku tymczasowego:
		if (entry.owner_pid != getpid())
		{
			if (write(tmpfiled, &entry, sizeof(entry)) < 1)
			{
				close(filed);
				close(tmpfiled);
				return -4;
			}
		}
	}
	close(filed);
	close(tmpfiled);

	// usun plik 'fd.tmp' i zmien nazwe tymczasowego na 'fd.tmp':
	if (remove(filename) != 0)
		return -1;
	if (rename(tmpname, filename) != 0)
		return -1;

	return 0;
}

int update_desc(int fd, int offset)
{
	// stworz nazwe "fd.tmp":
	char filename[16];
	if (createFilePath(filename, fd, 0) == NULL)
		return -1;

	// otworz plik fd.tmp:
	int filed = open(filename, O_RDWR);	// read-write: czytanie i pisanie w pliku, musi istniec
	if (filed < 0)
		return -2;

	// szukamy odpowiedniego wpisu i go modyfikujemy:
	lseek(filed, 0, SEEK_SET);
	while (1)
	{
		// odczytaj deskryptor:
		struct fd_entry entry;
		int readBytes = read(filed, &entry, sizeof(entry));
		// blad czy end-of-file?
		if (readBytes == 0)			// eof
			break;
		else if (readBytes < sizeof(entry))	// blad
		{
			close(filed);
			return -3;
		}
		if (entry.owner_pid == getpid())	// znalezlismy wpis
		{
			if (lseek(filed, -sizeof(entry), SEEK_CUR) < 0)
			{
				close(filed);
				return -4;
			}
			entry.rwoffset = offset;
			if (write(filed, &entry, sizeof(entry)) < 1)
			{
				close(filed);
				return -4;
			}
			close(filed);
			return 0;
		}
	}
	close(filed);

	return -1;
}

int get_desc(int fd, int* offset, char* mode)
{
	// stworz nazwe "fd.tmp":
	char filename[16];
	if (createFilePath(filename, fd, 0) == NULL)
		return -1;

	// otworz plik fd.tmp:
	int filed = open(filename, O_RDONLY);	// read binary: czytanie z pliku, plik musi istniec
	if (filed < 0)
		return -2;

	// wyszukaj wpisu z zadanym 'fd':
	lseek(filed, 0, SEEK_SET);
	while (1)
	{
		// odczytaj deskryptor:
		struct fd_entry entry;
		int readBytes = read(filed, &entry, sizeof(entry));
		// blad czy end-of-file?
		if (readBytes == 0)			// eof
			break;
		else if (readBytes < sizeof(entry))	// blad
		{
			close(filed);
			return -3;
		}
		// znaleziono deskryptor => wypisz dane i koncz:
		if (entry.owner_pid == getpid())
		{
			*offset = entry.rwoffset;
			*mode = entry.mode;
			close(filed);
			return 0;
		}
	}
	close(filed);
	return -1;
}

int del_descfile(int fd)
{
	// stworz nazwe "fd.tmp":
	char filename[16];
	if (createFilePath(filename, fd, 0) == NULL)
		return -1;
	if (remove(filename) != 0)
		return -1;
  	return 0;
}

