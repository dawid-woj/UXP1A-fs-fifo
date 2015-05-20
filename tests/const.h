#define LINK 1
#define UNLINK 2
#define TOKEN 3
#define ERROR 4
#define UNMOUNT 5

struct fifo_msg
{
	int type;		
	int code;

};

//char *initfifo_name = "fifos/initfifo";
char *initfifo_name = "initfifo";
