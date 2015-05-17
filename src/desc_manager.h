/*
DESC_MANAGER
Moduł odpowiedzialny za obsługę plików tymczasowych, w których zawarte są informacje na temat procesów,
które mają otwarte deskryptory plików (dla każdego otwartego pliku konieczne jest zapamiętanie rekordów
typu: pid, aktualna pozycja wskaźnika odczytu/zapisu, prawa z jakimi został otworzony plik).
*/

#ifndef DESCMANAGER_H_
#define DESCMANAGER_H_

const char TMP_DIR[] = "temp/";		// sciezka do folderu z plikami tymczasowymi


/*
Tworzy nowy wpis dla pliku o inode=fd w pliku tymczasowym  fd.tmp na rzecz wywołującego procedurę procesu.
Jeśli jest to pierwszy wpis, to tworzy nowy plik tymczasowy dla otwieranego pliku.
Zwraca: 0 – sukces, liczba ujemna – niepowodzenie.
Bledy:
-1 - inny blad
-2 - blad otwarcia/utworzenia pliku
-3 - blad zapisu do pliku
*/
int add_desc(int fd, char mode);

/*
Usuwa wpis dla pliku o inode=fd w pliku tymczasowym  fd.tmp na rzecz wywołującego procedurę procesu.
Jeśli jest to ostatni wpis, to plik tymczasowy zostanie usunięty.
Zwraca: 0 – sukces, liczba ujemna – niepowodzenie.
Bledy:
-1 - inny blad
-2 - blad otwarcia pliku
-3 - blad utworzenia pliku tymczasowego
-4 - blad zapisu do pliku
*/
int del_desc(int fd);

/*
Ustawia we wpisie dla pliku o inode=fd w pliku tymczasowym  fd.tmp na rzecz wywołującego procedurę procesu
wartość wskaźnika odczytu/zapisu do pliku.
Zwraca: 0 – sukces, liczba ujemna – niepowodzenie.
Bledy:
-1 - inny blad / nie ma takiego wpisu
-2 - blad otwarcia pliku
-3 - blad odczytu pliku
-4 - blad zapisu do pliku
*/
int update_desc(int fd, int offset);

/*
Umieszcza w offset aktualną wartość wskaźnika odczytu/zapisu a w mode tryb dostępu do pliku o inode=fd
pochodzące z pliku tymczasowego  fd.tmp dla wywołującego procedurę procesu.
Zwraca: 0 – sukces, liczba ujemna – niepowodzenie.
Bledy:
-1 - inny blad / nie ma takiego wpisu 
-2 - blad otwarcia pliku
-3 - blad odczytu pliku
*/
int get_desc(int fd, int* offset, char* mode);

/*
Usuwa plik tymczasowy  dla pliku o inode=fd.
Zwraca: 0 – sukces, -1 – niepowodzenie.
*/
int del_descfile(int fd);

#endif
