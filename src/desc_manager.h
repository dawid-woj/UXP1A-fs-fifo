#ifndef DESCMANAGER_H_
#define DESCMANAGER_H_

int add_desc(int fd, char mode);

int del_desc(int fd);

int update_desc(int fd, int offset);

int get_desc(int fd, int* offset, char* mode);

int del_descfile(int fd);

#endif
