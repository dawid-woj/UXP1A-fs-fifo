#ifndef SIMPLEFS_H_
#define SIMPLEFS_H_

#define SFS_RDONLY	1	// Bit 0
#define SFS_WRONLY	2	// Bit 1
#define SFS_RDWR	4	// Bit 2
#define SFS_CREAT	8	// Bit 3

int simplefs_make(char* name, int blocks_per_inode);

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

#endif
