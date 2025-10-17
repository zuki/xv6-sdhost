// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_UTIL_H
#define INC_NET_UTIL_H

#include <types.h>

/*
 * Array
 */

#define countof(x) ((sizeof(x) / sizeof(*x)))
#define tailof(x) (x + countof(x))
#define indexof(x, y) (((uintptr_t)y - (uintptr_t)x) / sizeof(*y))

/*
 * Time
 */

#define timeval_add_usec(x, y)         \
    do {                               \
        (x)->tv_sec += y / 1000000;    \
        (x)->tv_usec += y % 1000000;   \
        if ((x)->tv_usec >= 1000000) { \
            (x)->tv_sec += 1;          \
            (x)->tv_usec -= 1000000;   \
        }                              \
    } while(0);

#define timespec_add_nsec(x, y)           \
    do {                                  \
        (x)->tv_sec += y / 1000000000;    \
        (x)->tv_nsec += y % 1000000000;   \
        if ((x)->tv_nsec >= 1000000000) { \
            (x)->tv_sec += 1;             \
            (x)->tv_nsec -= 1000000000;   \
        }                                 \
    } while(0);

/*
 * Queue
 */

struct queue_entry;

struct queue_head {
    struct queue_entry *head;
    struct queue_entry *tail;
    unsigned int num;
};

void queue_init(struct queue_head *queue);
void *queue_push(struct queue_head *queue, void *data);
void *queue_pop(struct queue_head *queue);
void *queue_peek(struct queue_head *queue);
void queue_foreach(struct queue_head *queue, void (*func)(void *arg, void *data), void *arg);

/*
 * Byteorder
 */

uint16_t hton16(uint16_t h);
uint16_t ntoh16(uint16_t n);
uint32_t hton32(uint32_t h);
uint32_t ntoh32(uint32_t n);
uint64_t hton64(uint64_t h);
uint64_t ntoh64(uint64_t n);

// big endian (to be used for constants only)
#define BE(value)	((((value) & 0xFF00) >> 8) | (((value) & 0x00FF) << 8))

/*
 * Checksum
 */

uint16_t cksum16(uint16_t *addr, uint16_t count, uint32_t init);

#endif
