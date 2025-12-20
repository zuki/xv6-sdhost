
#ifndef INC_DRIVER_H
#define INC_DRIVER_H

#include <types.h>

#define DEVMAJOR_SD         1
#define DEVMAJOR_CONSOLE    2
#define DEVMAJOR_TTY        3

#define DEVNUM(major, minor)    ((major) << 8 | (minor & 0xff))

typedef unsigned int major_t;
typedef unsigned int minor_t;

struct driver {
    int (*init)(void);
    int (*open)(minor_t minor, int mode);
    int (*close)(minor_t minor);
    int (*read)(minor_t minor, char *buffer, off_t offset, size_t size);
    int (*write)(minor_t minor, const char *buffer, off_t offset, size_t size);
    int (*ioctl)(minor_t minor, unsigned int request, void *argp, uid_t uid);
    int (*poll)(minor_t minor, int events);
    off_t (*seek)(minor_t minor, off_t position, int whence, off_t offset);
};


int register_driver(major_t major, struct driver *driver);

int dev_open(device_t dev, int access);
int dev_close(device_t dev);
int dev_read(device_t dev, char *buffer, off_t offset, size_t size);
int dev_write(device_t dev, const char *buffer, off_t offset, size_t size);
int dev_ioctl(device_t dev, unsigned int request, void *argp, uid_t uid);
int dev_poll(device_t dev, int events);
off_t dev_seek(device_t dev, off_t position, int whence, off_t offset);


#endif
