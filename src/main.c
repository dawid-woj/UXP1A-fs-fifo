/*
To ponizsze moze realizowac rozne scenariusze w zaleznosci od parametrow wywolan, np.
gdy skompilujemy ten program (gcc -o main main.c) i wywolamy:
./main --newsfs system 20 --mount system --mkfile pliczek 4 --sleep 3 --open pliczek 2 --cptosfs cos 20 --close --umount [&]

to wykona sie, co nastepuje:
1) simplefs_make() - nowy wirtualny dysk o nazwie 'system' z 20 blokami na inode
2) simplefs_mount() - zamontuj system z pliku 'system'
3) simplefs_creat() - stworz plik 'pliczek' z mode=4
4) usnij na 3 sekundy
5) otworz plik 'pliczek' z mode=2
6) zapisz do otwartego pliku (czyli 'pliczek') 20 bajtow z pliku 'cos' bedace na zewnatrz systemu
7) zamknij otwarty plik ('pliczek')
8) odmontuj system

UWAGA!!!
Jesli chcesz przekazac ujemny argument, np. -5 nalezy pisac m5!!!
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "simplefs.h"
#include "simplefs_aux.h"
#include "sfs_vd.h"

/* Polecenia:
 * --newsfs filename bpi 	OK
 * --mount filename		OK
 * --umount			OK
 * --mkfile path mode		OK
 * --mkdir path			OK
 * --rm path			?
 * --open path mode		OK
 * --close			OK
 * --seek whence offset
 * --cptosfs path bytes		OK
 * --cpfromsfs path bytes	OK
 * --sleep value			OK
 * -------------------------
 * --listdir path
 * --stat filename
 * --printblock blockid range
 * --printinodes
 */


int cptosfs(int fd, const char * filename, int bytes);
int cpfromsfs(int fd, const char * filename, int bytes);


int isNextOption(const char * str)
{
	if (str == 0)
		return 1;
	char c1 = str[0];
	char c2 = str[1];
	if (c1 == '-' && c2 == '-')
		return 1;
	return 0;
}


long int strToNumber(char * str)
{
	if (str == 0)
		return 0;
	if (str[0] == 'm') // 'm' zastepuje znak minus dla ujemnych wartosci
		str[0] = '-';
	return strtol(str, NULL, 10);
}



int main(int argc, char **argv)
{
	const struct option long_options[] = {
		{"newsfs", 1, NULL, 'n'},
		{"mount", 1, NULL, 'm'},
		{"umount", 0, NULL, 'u'},
		{"mkfile", 1, NULL, 'f'},
		{"mkdir", 1, NULL, 'd'},
		{"rm", 1, NULL, 'r'},
		{"open", 1, NULL, 'o'},
		{"close", 0, NULL,'c'},
		{"seek", 1, NULL,'s'},
		{"cptosfs", 1, NULL, 't'},
		{"cpfromsfs", 1, NULL, 'p'},
		{"sleep", 1, NULL, 'w'},
		{"listdir", 1, NULL, 'l'},
		{"stat", 1, NULL, 'a'},
		{"printblock", 1, NULL, 'b'},
		{"printinodes", 0, NULL, 'i'},
		{0, 0, 0, 0}
	};
	opterr = 0;
	int fd = -1;
	int c, error = 0;
	int status; // Zmienna zapamietujaca status wykonania operacji simplefs 

	// skopiuj tablice argv (bo chcemy uniknac permutacji argv po przejsciu getopt):
	char **args = (char **)malloc(argc * sizeof(char*));
	int i = 0;
	while (i < argc) {
		args[i] = argv[i];
		++i;
	}
	
	// pierwsze przejscie po argumentach w celu sprawdzenia ich poprawnosci:
	while (1) {
		// wczytanie kolejnej opcji z argv:
		c = getopt_long(argc, args, "", long_options, NULL);
		// gdy nie ma wiecej opcji do wczytania -> zakoncz:
		if (c == -1)
			break;

		switch (c) {
			// newsfs path bpi
			case 'n':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// mount path
			case 'm':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// umount
			case 'u':
				break;
			// mkfile filename mode
			case 'f':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// mkdir dirname
			case 'd':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// rm filename
			case 'r':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// open filename mode
			case 'o':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// close
			case 'c':
				break;
			// seek whence offset
			case 's':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// cptosfs filename value
			case 't':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// cpfromsfs filename value
			case 'p':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;
				}
				break;
			// sleep seconds
			case 'w':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;;
				}
				break;
			// listdir path
			case 'l':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;;
				}
				break;
			// stat filename
			case 'a':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;;
				}
				break;
			// printblock blockid range
			case 'b':
				if (isNextOption(optarg) || isNextOption(argv[optind])) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;;
				}
				break;
			// printinodes
			case 'i':
				break;
			default:
				fprintf(stderr, "%s: bledna opcja!\n", args[optind]);
				error = 1;
		}
		if (error) {
			free(args);
			return -1;
		}
	}

	optind = 1;

	// drugie przejscie: wszystkie argumenty sa poprawne, wykonaj je po kolei:
	while (1) {
		// wczytanie kolejnej opcji z argv:
		c = getopt_long(argc, argv, "", long_options, NULL);
		// gdy nie ma wiecej opcji do wczytania -> zakoncz:
		if (c == -1)
			break;

		switch (c) {
			// newsfs path bpi
			case 'n':
				status = simplefs_make(optarg, (int)strToNumber(argv[optind]));
				printf("simplefs_make(%s, %d)\n", optarg, (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				++optind;
				break;
			// mount path
			case 'm':
				status = simplefs_mount(optarg);
				printf("simplefs_mount(%s)\n", optarg);
				printf("Operation status = %d\n", status);
				break;
			// umount
			case 'u':
				status = simplefs_umount();
				printf("simplefs_umount()\n");
				printf("Operation status = %d\n", status);
				break;
			// mkfile filename mode
			case 'f':
				status = fd = simplefs_creat(optarg, (int)strToNumber(argv[optind]));
				printf("simplefs_creat(%s, %d)\n", optarg, (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				++optind;
				break;
			// mkdir dirname
			case 'd':
				status = simplefs_mkdir(optarg);
				printf("simplefs_mkdir(%s)\n", optarg);
				printf("Operation status = %d\n", status);
				break;
			// rm filename
			case 'r':
				status = simplefs_unlink(optarg);
				printf("simplefs_unlink(%s)\n", optarg);
				printf("Operation status = %d\n", status);
				break;
			// open filename mode
			case 'o':
				status = fd = simplefs_open(optarg, (int)strToNumber(argv[optind]));
				printf("simplefs_open(%s, %d)\n", optarg, (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				++optind;
				break;
			// close
			case 'c':
				status = simplefs_close(fd);
				printf("simplefs_close(%d)\n", fd);
				printf("Operation status = %d\n", status);
				break;
			// seek whence offset
			case 's':
				status = simplefs_lseek(fd, (int)strToNumber(optarg), (int)strToNumber(argv[optind]));
				printf("simplefs_lseek(%d, %d, %d)\n", fd, (int)strToNumber(optarg), (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				++optind;
				break;
			// cptosfs filename value
			case 't':
				status = cptosfs(fd, optarg, (int)strToNumber(argv[optind]));
				printf("cptosfs(%d, %s, %d)\n", fd, optarg, (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				++optind;
				break;
			// cpfromsfs filename value
			case 'p':
				status = cpfromsfs(fd, optarg, (int)strToNumber(argv[optind]));
				printf("cpfromsfs(%d, %s, %d)\n", fd, optarg, (int)strToNumber(argv[optind]));
				printf("Operation status = %d\n", status);
				break;
			// sleep seconds
			case 'w':
				sleep((int)strToNumber(optarg));
				break;
			// listdir path
			case 'l':
				printf("Directory \"%s\" content\n", optarg);
				debug__list_dir(optarg);
				break;
			// stat filename
			case 'a':
				printf("Sfsfile \"%s\" statistics\n", optarg);
				debug__analyse_sfsfile(optarg);
				break;
			// printblock blockid range
			case 'b':
				debug__print_block((unsigned short)strToNumber(optarg), (int)strToNumber(argv[optind]));
				break;
			// printinodes
			case 'i':
				debug__print_used_inodes();
				break;
			default:
				break;
		}
	}

	free(args);
	
	return 0;
}

int cptosfs(int fd, const char * filename, int bytes)
{
  int extfd, res;
  char* buf = (char*) malloc(bytes);
  
  extfd =  open(filename, O_RDONLY);
  read(extfd, buf, bytes);
  res = simplefs_write(fd, buf, bytes);
  free(buf);
  close(extfd);
  return res;
}

int cpfromsfs(int fd, const char * filename, int bytes)
{
  int extfd, res;
  char* buf = (char*) malloc(bytes);
  
  extfd =  open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
  res = simplefs_read(fd, buf, bytes);
  write(extfd, buf, bytes);
  free(buf);
  close(extfd);
  return res;
}
