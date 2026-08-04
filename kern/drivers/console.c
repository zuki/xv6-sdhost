#include <console.h>

#include <types.h>
#include <driver.h>
#include <arm.h>
#include <uart.h>
#include <irq.h>
#include <spinlock.h>
#include <mm.h>
#include <string.h>
#include <vfs.h>

struct spinlock dbglock;
static int panicked = -1;

// Driver Definition
int console_init(void);
int console_open(minor_t minor, int mode);
int console_close(minor_t minor);
int console_read(minor_t minor, char *buffer, off_t offset, size_t size);
int console_write(minor_t minor, const char *buffer, off_t offset, size_t size);
int console_ioctl(minor_t minor, unsigned int request, void *argp, uid_t uid);
int console_poll(minor_t minor, int events);
off_t console_seek(minor_t minor, off_t position, int whence, off_t offset);

struct driver console_driver = {
    console_init,
    console_open,
    console_close,
    console_read,
    console_write,
    console_ioctl,
    console_poll,
    console_seek,
};

#define INPUT_BUF 128
struct serial_channel {
    char buf[INPUT_BUF];
    size_t r;                   // Read index
    size_t w;                   // Write index
    size_t e;                   // Edit index
    int opens;
    int open_mode;
    struct spinlock lock;
} channel;

#define C(x)  ((x)-'@')         // Control-x
#define BACKSPACE 0x100

static void consputc(int c)
{
    if (c == BACKSPACE) {
        uart_putchar('\b');
        uart_putchar(' ');
        uart_putchar('\b');
    } else {
        uart_putchar(c);
    }
}

static void console_intr1(int (*getc)())
{
    int c, prof = 0;

    acquire(&channel.lock);
    if (panicked >= 0) {
        release(&channel.lock);
        while (1) ;
    }

    while ((c = getc()) >= 0) {
        switch (c) {
        case C('P'):           // Process listing.
            prof = 1;
            break;
        case C('U'):           // Kill line.
            while (channel.e != channel.w
                   && channel.buf[(channel.e - 1) % INPUT_BUF] != '\n') {
                channel.e--;
                consputc(BACKSPACE);
            }
            break;
        case C('H'):
        case '\x7f':           // Backspace
            if (channel.e != channel.w) {
                channel.e--;
                consputc(BACKSPACE);
            }
            break;
        default:
            if (c != 0 && channel.e - channel.r < INPUT_BUF) {
                c = (c == '\r') ? '\n' : c;
                channel.buf[channel.e++ % INPUT_BUF] = c;
                consputc(c);
                if (c == '\n' || c == C('D')
                    || channel.e == channel.r + INPUT_BUF) {
                    channel.w = channel.e;
                    wakeup(&channel.r);
                }
            }
            break;
        }
    }
    release(&channel.lock);

    if (prof) {
        //mm_dump();
        procdump();
    }
}

void console_intr(void *_p)
{
    console_intr1(uart_getchar);
}

static void printint(int64_t x, int base, int sign, int zero, int col)
{
    static char digit[] = "0123456789abcdef";
    static char buf[64];

    if (sign && x < 0) {
        x = -x;
        uart_putchar('-');
    }

    int i = 0;
    uint64_t t = x;
    do {
        buf[i++] = digit[t % base];
    } while (t /= base);

    for (; i < col; i++)
        if (zero == 1)
            buf[i] = '0';
        else if (zero == -1)
            buf[i] = ' ';
        else break;

    while (i--)
        uart_putchar(buf[i]);
}

void vprintfmt(void (*putch)(int), const char *fmt, va_list ap)
{
    int i, c;
    char *s;

    if (panicked >= 0 && panicked != cpuid()) {
        release(&channel.lock);
        while (1) ;
    }

    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            putch(c);
            continue;
        }

        int n = 0;
        int z = 0;
        if (fmt[i+1] == '0') {
            z = 1;
            i++;
        } else if (fmt[i+1] == '-') {
            z = -1;
            i++;
        }

        for (; fmt[i+1] >= '0' && fmt[i+1] <= '9'; i++) {
            n = n * 10 + (fmt[i+1] - '0') % 10;
        }

        int l = 0;
        for (; fmt[i + 1] == 'l'; i++)
            l++;

        if (!(c = fmt[++i] & 0xff))
            break;

        switch (c) {
        case 'u':
            if (l > 0)
                printint(va_arg(ap, uint64_t), 10, 0, z, n);
            else
                printint(va_arg(ap, uint32_t), 10, 0, z, n);
            break;
        case 'd':
            if (l > 0)
                printint(va_arg(ap, int64_t), 10, 1, z, n);
            else
                printint(va_arg(ap, int), 10, 1, z, n);
            break;
        case 'x':
            if (l > 0)
                printint(va_arg(ap, uint64_t), 16, 0, z, n);
            else
                printint(va_arg(ap, uint32_t), 16, 0, z, n);
            break;
        case 'p':
            putch('0'); putch('x');
            printint((uint64_t) va_arg(ap, void *), 16, 0, z, n-2);
            break;
        case 'c':
            putch(va_arg(ap, int));
            break;
        case 's':
            if ((s = (char *)va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                putch(*s);
            break;
        case '%':
            putch('%');
            break;
        default:
            /* Print unknown % sequence to draw attention. */
            putch('%');
            putch(c);
            break;
        }
    }
}

/* Print to the console. */
void cprintf(const char *fmt, ...)
{
    va_list ap;

    acquire(&channel.lock);
    va_start(ap, fmt);
    vprintfmt(uart_putchar, fmt, ap);
    va_end(ap);
    release(&channel.lock);
}

/* Caller should hold channel.lock. */
void cprintf1(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintfmt(uart_putchar, fmt, ap);
    va_end(ap);
}


void panic(const char *fmt, ...)
{
    va_list ap;

    acquire(&channel.lock);
    if (panicked < 0)
        panicked = cpuid();
    else {
        release(&channel.lock);
        while (1) ;
    }
    va_start(ap, fmt);
    vprintfmt(uart_putchar, fmt, ap);
    va_end(ap);
    release(&channel.lock);

    cprintf("%s:%d: kernel panic at cpu %d.\n", __FILE__, __LINE__,
            cpuid());
    while (1) ;
}

void hexdump(const void *data, size_t size, const char *name)
{
    unsigned char *src;
    int offset, index;

    src = (unsigned char *)data;
    cprintf("=== %s dump ===\n", name ? name : "");
    cprintf("+------------------+-------------------------------------------------+------------------+\n");
    for (offset = 0; offset < (int)size; offset += 16) {
        cprintf("| 0x%016llx | ", (uint64_t)src + offset);
        for (index = 0; index < 16; index++) {
            if (offset + index < (int)size) {
                cprintf("%02x ", 0xff & src[offset + index]);
            } else {
                cprintf("   ");
            }
        }
        cprintf("| ");
        for (index = 0; index < 16; index++) {
            if (offset + index < (int)size) {
                if (isascii(src[offset + index]) && isprint(src[offset + index])) {
                    cprintf("%c", src[offset + index]);
                } else {
                    cprintf(".");
                }
            } else {
                cprintf(" ");
            }
        }
        cprintf(" |\n");
    }
    cprintf("+------------------+-------------------------------------------------+------------------+\n\n");

}

int console_preinit(void) {
    // mini UARTを使用
    uart_init();

    irq_enable(IRQ_AUX);
    irq_register(IRQ_AUX, console_intr, 0);
    info("console_preinit ok");
    return 0;
}

int console_init(void)
{
    int err = register_driver(DEVMAJOR_CONSOLE, &console_driver);
    info("console_init ok");
    return err;
}

int console_open(minor_t minor, int mode)
{
    channel.opens++;
    channel.open_mode = mode;
    return 0;
}

int console_close(minor_t minor)
{
    channel.opens--;
    channel.open_mode = 0;
    return 0;
}

int console_read(minor_t minor, char *buffer, off_t offset, size_t size)
{
    //char *p = buffer;
    int count = 0;
    acquire(&channel.lock);
    while (size > 0) {
        while (channel.r == channel.w) {
            if (thisproc()->killed) {
                release(&channel.lock);
                return -1;
            }
            sleep(&channel.r, &channel.lock);
        }
        int c = channel.buf[channel.r++ % INPUT_BUF];
        if (c == C('D')) {      // EOF
            if (size < (size_t)buffer) {
                // Save ^D for next time, to make sure
                // caller gets a 0-byte result.
                channel.r--;
            }
            break;
        }
        *buffer++ = c;
        --size;
        ++count;
        if (c == '\n')
            break;
    }
    //debug("buffer: 0x%x, size: 0x%x, buffer: %s, r: %d, w: %d, e: %d, buffer-p: %d", buffer, size, p, channel.r, channel.w, channel.e, (size_t)(buffer - p));

    release(&channel.lock);
    //return (int)(buffer - p);
    return count;
}

int console_write(minor_t minor, const char *buffer, off_t offset, size_t size)
{
    acquire(&channel.lock);
    for (size_t i = 0; i < size; i++)
        consputc(buffer[i] & 0xff);
    release(&channel.lock);

    return size;
}

int console_ioctl(minor_t minor, uint32_t request, void *argp, uid_t uid)
{
    return 0;
}

int console_poll(minor_t minor, int events)
{
    return 0;
}

off_t console_seek(minor_t minor, off_t position, int whence, off_t offset)
{
    return 0;
}
