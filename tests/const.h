#define LINK 0
#define UNLINK 1
#define TOKEN 2
#define ERROR 3
#define UNMOUNT 4

struct fifo_msg
{
	int type;		
	int code;
	int code_2;
};

char *initfifo_name = "/home/szymon/Dokumenty/SFS UXP1A/UXP1A-fs-fifo/tests/initfifo";
