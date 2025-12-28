#include <types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <string.h>
#include <linux/termios.h>
#include <linux/ioctl.h>
#include <net/sockio.h>
#include <vfs.h>
#include <driver.h>
#include <linux/syscall.h>
#include <linux/signal.h>
#include <proc.h>
#include <spinlock.h>
#include <console.h>

#define TTY_DEVICE_NUM  2

#define MAX_CANNON      256

struct tty_device {
    device_t rdev;
    pid_t pgid;

    struct termios tio;

    int opens;
    struct spinlock lock;
};

struct termios default_tio = {
    .c_iflag = (BRKINT | ICRNL | IXON | IXANY | IMAXBEL),
    .c_oflag = (OPOST | ONLCR),
    .c_cflag = (CREAD | CS8 | B115200),
    .c_lflag = (ISIG | IEXTEN | ICANON | ECHO | ECHOE | ECHOCTL | ECHOKE),
    .c_cc = {
        '\x04',         // VEOF
        0xff,           // VEOL
        '\x08',         // VERASE
        '\x03',         // VINTR
        '\x15',         // VKILL
        1,              // VMIN
        '\x1C',         // VQUIT
        0,              // VTIME
        '\x1A',         // VSUSP
        '\x13',         // VSTART
        '\x11',         // VSTOP
    }
};

// Driver Definition
int tty_init(void);
int tty_open(minor_t minor, int access);
int tty_close(minor_t minor);
int tty_read(minor_t minor, char *buffer, off_t offset, size_t size);
int tty_write(minor_t minor, const char *buffer, off_t offset, size_t size);
int tty_ioctl(minor_t minor, unsigned int request, void *argp, uid_t uid);
int tty_poll(minor_t minor, int events);
off_t tty_seek(minor_t minor, off_t position, int whence, off_t offset);

struct driver tty_driver = {
    tty_init,
    tty_open,
    tty_close,
    tty_read,
    tty_write,
    tty_ioctl,
    tty_poll,
    tty_seek,
};


static struct tty_device devices[TTY_DEVICE_NUM];

int tty_init(void)
{

    for (short i = 0; i < TTY_DEVICE_NUM; i++) {
        devices[i].rdev = makedev(DEVMAJOR_CONSOLE, i);
        devices[i].pgid = 0;
        devices[i].opens = 0;
        initlock(&devices[i].lock, "ttylock");
        memcpy(&devices[i].tio, &default_tio, sizeof(struct termios));
    }

    int err = register_driver(DEVMAJOR_TTY, &tty_driver);
    info("tty_init ok");
    return err;
}

// minorは1から、devicesは0からなので (minor - 1) を使う
int tty_open(minor_t minor, int mode)
{
    if ((minor - 1) >= TTY_DEVICE_NUM)
        return -ENODEV;
    if (devices[minor-1].opens++ == 0)
        return dev_open(devices[minor-1].rdev, mode | O_NONBLOCK | O_EXCL);
    return 0;
}

int tty_close(minor_t minor)
{
    if ((minor - 1) >= TTY_DEVICE_NUM)
        return -ENODEV;
    if (devices[minor-1].opens == 0)
        return -EBADF;
    if (--(devices[minor-1].opens) == 0)
        return dev_close(devices[minor-1].rdev);
    return 0;
}

int tty_read(minor_t minor, char *buffer, off_t offset, size_t size)
{
    int read;
    if ((minor - 1) >= TTY_DEVICE_NUM)
        return -ENODEV;

    read = dev_read(devices[minor-1].rdev, buffer, offset, size);
    return read;
}

int tty_write(minor_t minor, const char *buffer, off_t offset, size_t size)
{
    int written;

    if ((minor - 1) >= TTY_DEVICE_NUM)
        return -ENODEV;

    written = dev_write(devices[minor-1].rdev, buffer, offset, size);
    return written;
}

int tty_ioctl(minor_t minor, unsigned int request, void *argp, uid_t uid)
{
    if ((minor - 1) >= TTY_DEVICE_NUM)
        return -ENODEV;

    // TODO: さらにrequestに対応
    switch (request) {
        case TCGETS: {
            acquire(&devices[minor-1].lock);
            memcpy((struct termios *) argp, &devices[minor-1].tio, sizeof(struct termios));
            release(&devices[minor-1].lock);
            return 0;
        }
        case TCSETS: {
            acquire(&devices[minor-1].lock);
            memcpy(&devices[minor-1].tio, (struct termios *) argp, sizeof(struct termios));
            release(&devices[minor-1].lock);
            return 0;
        }

        case TIOCGWINSZ:
            if (argp == NULL) return -EINVAL;
            ((struct winsize *)argp)->ws_row = 24;
            ((struct winsize *)argp)->ws_col = 80;
            return 0;
        case TIOCSWINSZ:
            // Windowサイズ設定: 当面何もしない
            return 0;

        case TIOCGPGRP: {
            *((int *) argp) = devices[minor-1].pgid;
            return 0;
        }
        case TIOCSPGRP: {
            devices[minor-1].pgid = *((int *) argp);
            return 0;
        }
        default:
            return dev_ioctl(devices[minor-1].rdev, request, argp, uid);
    }
    return -EINVAL;
}

int tty_poll(minor_t minor, int events)
{
    return 0;
}

off_t tty_seek(minor_t minor, off_t position, int whence, off_t offset)
{
    return 0;
}
