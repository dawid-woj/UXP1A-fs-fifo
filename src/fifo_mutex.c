#include "fifo_mutex.h"

#define LINK 			0
#define UNLINK 			1
#define TOKEN			2
#define ERROR 			3
#define UNMOUNT_PREPARE		4
#define UNMOUNT_EXECUTE		5

/***************************************************************************
 * Wewnetrzne struktury danych
 **************************************************************************/

struct fifo_msg
{
  int type;
  int code;
};

/***************************************************************************
 * API modulu
 **************************************************************************/

int fifomutex_lock()
{
  return 0;
}

int fifomutex_unlock()
{
  return 0;
}

int fifomutex_unmount()
{
  return 0;
}

int fsinit()
{
  return 0;
}
