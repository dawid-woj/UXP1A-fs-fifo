/*
To ponizsze moze realizowac rozne scenariusze w zaleznosci od parametrow wywolan, np.
gdy skompilujemy ten program (gcc -o main main.c) i wywolamy:
./main -newsfs system -mount system -mkfile dupa 4 -sleep 3 -open dupa 2 -cptosfs cos 20 -close -umount [&]

to wykona sie, co nastepuje:
1) simplefs_make() - nowy wirtualny dysk o nazwie 'system'
2) simplefs_mount() - zamontuj system z pliku 'system'
3) simplefs_creat() - stworz plik 'dupa' z mode=4
4) usnij na 3 sekundy
5) otworz plik 'dupa' z mode=2
6) zapisz do otwartego pliku (czyli 'dupa') 20 bajtow z pliku 'cos' bedace na zewnatrz systemu
7) zamknij otwarty plik ('dupa')
8) odmontuj system
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>



int cptosfs(int fd, const char * filename, int bytes);
int cpfromsfs(int fd, const char * filename, int bytes);


int isNextOption(const char * str)
{
	char c = str[0];
	if (c == '-')
		return 1;
	return 0;
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
		{"sleep", 1, NULL, 'l'},
		{0, 0, 0, 0}
	};
	opterr = 0;
	int fd = -1;
	int c, error = 0;

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
		c = getopt_long_only(argc, args, "", long_options, NULL);
		// gdy nie ma wiecej opcji do wczytania -> zakoncz:
		if (c == -1)
			break;

		switch (c) {
			// newsfs path
			case 'n':
				if (isNextOption(optarg)) {
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
			// seek value whence
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
			case 'l':
				if (isNextOption(optarg)) {
					fprintf(stderr, "%s: brak argumentu(ow)!\n", args[optind-2]);
					error = 1;;
				}
				break;
			default:
				fprintf(stderr, "%s: bledna opcja!\n", args[optind-1]);
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
		c = getopt_long_only(argc, argv, "", long_options, NULL);
		// gdy nie ma wiecej opcji do wczytania -> zakoncz:
		if (c == -1)
			break;

		switch (c) {
			// newsfs path
			case 'n':
				//simplefs_make(optarg);
				printf("simplefs_make(%s)\n", optarg);
				break;
			// mount path
			case 'm':
				//simplefs_mount(optarg);
				printf("simplefs_mount(%s)\n", optarg);
				break;
			// umount
			case 'u':
				//simplefs_umount();
				printf("simplefs_umount()\n");
				break;
			// mkfile filename mode
			case 'f':
				//simplefs_creat(optarg, (int)strtol(argv[optind], NULL, 10));
				printf("simplefs_creat(%s, %d)\n", optarg, (int)strtol(argv[optind], NULL, 10));
				++optind;
				break;
			// mkdir dirname
			case 'd':
				//simplefs_mkdir(optarg);
				printf("simplefs_mkdir(%s)\n", optarg);
				break;
			// rm filename
			case 'r':
				//simplefs_unlink(optarg);
				printf("simplefs_unlink(%s)\n", optarg);
				break;
			// open filename mode
			case 'o':
				//fd = simplefs_open(optarg, (int)strtol(argv[optind], NULL, 10));
				printf("simplefs_open(%s, %d)\n", optarg, (int)strtol(argv[optind], NULL, 10));
				++optind;
				break;
			// close
			case 'c':
				//simplefs_close(fd);
				printf("simplefs_close(fd)\n");
				break;
			// seek value whence
			case 's':
				//simplefs_lseek(fd, (int)strtol(argv[optind], NULL, 10), (int)strtol(optarg, NULL, 10));
				printf("simplefs_lseek(fd, %d, %d)\n", (int)strtol(argv[optind], NULL, 10), (int)strtol(optarg, NULL, 10));
				++optind;
				break;
			// cptosfs filename value
			case 't':
				//cptosfs(fd, optarg, (int)strtol(argv[optind], NULL, 10));
				printf("cptosfs(fd, %s, %d)\n", optarg, (int)strtol(argv[optind], NULL, 10));
				++optind;
				break;
			// cpfromsfs filename value
			case 'p':
				//cpfromsfs(fd, optarg, (int)strtol(argv[optind], NULL, 10));
				printf("cpfromsfs(fd, %s, %d)\n", optarg, (int)strtol(argv[optind], NULL, 10));
				break;
			// sleep seconds
			case 'l':
				sleep((int)strtol(optarg, NULL, 10));
				break;
			default:
				break;
		}
	}

	free(args);
	
	return 0;
}
