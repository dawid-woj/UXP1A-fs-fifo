#ifndef SIMPLEFS_H_
#define SIMPLEFS_H_

/* Flagi podawane jako argument */
#define SFS_RDONLY		1	// Odczyt (Bit 0)
#define SFS_WRONLY		2	// Zapis (Bit 1)
#define SFS_RDWR		3	// Odczyt i zapis (Bit 0 + 1)
#define SFS_CREAT		4	// Flaga tworzenia pliku (Bit 2)
#define SFS_SEEK_SET		0	// Poczatek pliku
#define SFS_SEEK_CUR		1	// Aktualna pozyca w pliku
#define SFS_SEEK_END		2	// Koniec pliku	

/* Kody sygnalizacyjne */
#define SFS_OK			0	// Kod sukcesu
#define SFS_EOF			0	// Koniec pliku
#define SFS_NOT_FOUND		-1	// Nie znaleziono pliku
#define SFS_NAME_TO_LONG 	-2	// Nazwa pliku/katalogu ma ponad 26 znakow
#define SFS_BAD_PATH		-3	// Niepoprawna sciezka do pliku
#define SFS_BAD_NAME		-4	// Niepoprawna nazwa pliku
#define SFS_TO_MANY_FILES	-5	// Brak wolnych inodow
#define SFS_DIR_FULL		-6	// Brak miejsca w katalogu
#define SFS_EACCESS		-7	// Blad praw dostepu do pliku
#define SFS_EDESC		-8	// Blad pliku tymczasowgo pliku dekskryptora, Uwaga! Jesli plik byl tworzony, to udalo sie go stworzyc, ale nie otworzyc
#define SFS_EOPENED		-9	// Plik jest juz otwarty
#define SFS_EEXISTS		-10	// Plik/katalog juz istnieje
#define SFS_NOSPACE		-11	// Brak miejsca na wirtualnym dysku
#define SFS_ECLOSED		-12	// Plik zamkniety
#define SFS_BAD_OPTION		-13	// Niepoprawny argument
#define SFS_BAD_DESC		-14	// Niepoprawny deskryptor pliku

int simplefs_make(char* name, int blocks_per_inode);

int simplefs_mount(char* name);

int simplefs_umount(void);

int simplefs_open(char *name, int mode);

int simplefs_unlink(char *name);

int simplefs_mkdir(char *name);

int simplefs_creat (char *name, int mode);

int simplefs_read(int fd, char *buf, int len);

int simplefs_write(int fd, char *buf, int len);

int simplefs_lseek(int fd, int whence, int offset);

int simplefs_close(int fd);

#endif
