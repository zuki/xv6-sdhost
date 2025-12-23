
#include <types.h>
#include <linux/errno.h>
#include <driver.h>
#include <console.h>

#define MAX_DRIVERS     6

static struct driver *drv_table[MAX_DRIVERS];

int register_driver(major_t major, struct driver *driver)
{
    if (major >= MAX_DRIVERS)
        return -1;

    drv_table[major] = driver;
    debug("drv_table[%d] = 0x%x", major, driver);
    return 0;
}

int dev_open(device_t dev, int access)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->open(minor, access);
}

int dev_close(device_t dev)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->close(minor);
}

int dev_read(device_t dev, char *buffer, off_t offset, size_t size)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->read(minor, buffer, offset, size);
}

int dev_write(device_t dev, const char *buffer, off_t offset, size_t size)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->write(minor, buffer, offset, size);
}

int dev_ioctl(device_t dev, unsigned int request, void *argp, uid_t uid)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->ioctl(minor, request, argp, uid);
}

int dev_poll(device_t dev, int events)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->poll(minor, events);
}

off_t dev_seek(device_t dev, off_t position, int whence, off_t offset)
{
    major_t major = major(dev);
    minor_t minor = minor(dev);

    if (major >= MAX_DRIVERS)
        return -ENXIO;
    return drv_table[major]->seek(minor, position, whence, offset);
}
