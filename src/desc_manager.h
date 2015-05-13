#ifndef DESCMANAGER_H_
#define DESCMANAGER_H_
#include <stdio.h>



int add_desc(int fd, char mode);

int del_desc(int fd);

int update_desc(int fd, int offset);

int get_desc(int fd, int* offset, char* mode);

#endif
