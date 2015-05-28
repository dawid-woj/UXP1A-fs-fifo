#ifndef SFS_OPS_H_
#define SFS_OPS_H_

int sfsop_open(char *name, int mode);

int sfsop_unlink(char *name);

int sfsop_mkdir(char *name);

int sfsop_read(int fd, char *buf, int len);

int sfsop_write(int fd, char *buf, int len);

int sfsop_lseek(int fd, int whence, int offset);

int sfsop_close(int fd);

#endif
