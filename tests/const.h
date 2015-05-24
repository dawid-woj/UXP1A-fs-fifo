#define LINK 1
#define UNLINK 2
#define TOKEN 3
#define ERROR 4
#define UNMOUNT 5
#define UNMOUNT_PREPARE 6
#define UNMOUNT_EXECUTE 7
#define NO_WRITERS 8

struct fifo_msg
{
	int type;		
	int code;

};

//char *initfifo_name = "fifos/initfifo";
char *initfifo_name = "initfifo";
