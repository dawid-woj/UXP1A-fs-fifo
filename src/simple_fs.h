#ifndef SIMPLE_FS_H_
#define SIMPLE_FS_H_
#include <stdio.h>


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

int make(char* name, int inods);

#endif
