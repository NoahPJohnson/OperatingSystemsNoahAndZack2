#ifndef VDI_UNIX_CALLS_H
#define VDI_UNIX_CALLS_H

#include <sys/types.h>
#include <unistd.h>

int vdi_open(const char *pathname);

int vdi_close();

ssize_t vdi_read(/*int fd,*/ void *buf, size_t count);

ssize_t vdi_write(int fd, const void *buf, size_t count);

off_t vdi_lseek(off_t offset, int whence);

void setHeader();

int translate();


#endif