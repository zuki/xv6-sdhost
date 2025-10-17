// From https://github.com/pandax381/xv6-riscv-net
#include <types.h>
#include <arm.h>
#include <fs.h>
#include <syscall.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <file.h>
#include <proc.h>
#include <console.h>
#include <net/socket.h>
#include <linux/errno.h>
#include <net/dns.h>

// socket(int domain, int type, int protocol);
// TODO: アクセス権限チェック
long sys_socket(void)
{
    int domain, type, protocol;

    if (argint(0, &domain) < 0 || argint(1, &type) < 0
     || argint(2, &protocol) < 0)
        return -EINVAL;
    trace("domain: %d, type: %d, protocol: %d", domain, type, protocol);
    return socket_alloc(domain, type, protocol);
}

// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
long sys_bind(void)
{
    struct file *f;
    struct sockaddr *addr;
    int addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;

    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argint(2, &addrlen) < 0)
        return -EINVAL;

    return socket_bind(f->socket, addr, addrlen);
}

// ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
long sys_recvfrom(void)
{
    struct file *f;
    char *buf;
    int flags;
    size_t len;
    struct sockaddr *addr;
    int *addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argu64(2, &len) < 0)
        return -EINVAL;

    if (argptr(1, (void **)&buf, len) < 0)
        return -EFAULT;

    if (argint(3, &flags) < 0)
        return -EINVAL;

    if (argptr(4, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argptr(5, (void **)&addrlen, sizeof(int)) <0)
        return -EFAULT;

    if (flags) {
        warn("flags: 0x%x not supported", flags);
        return -EINVAL;
    }

    if (f->socket->type == SOCK_DGRAM)
        return socket_recvfrom(f->socket, buf, len, addr, addrlen);
    else if (f->socket->type == SOCK_STREAM)
        return socket_read(f->socket, buf, len);
    else
        return -EINVAL;
}

// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
long sys_sendto(void)
{
    struct file *f;
    char *buf;
    int flags;
    size_t len;
    struct sockaddr *addr;
    int addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argu64(2, &len) < 0)
        return -EINVAL;

    if (argptr(1, (void **)&buf, len) < 0)
        return -EFAULT;

    if (argint(3, &flags) <0)
        return -EINVAL;

    if (argptr(4, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argint(5, &addrlen) < 0)
        return -EFAULT;

    if (flags) {
        warn("flags: 0x%x not supported", flags);
        return -EOPNOTSUPP;
    }

    if (f->socket->type == SOCK_DGRAM)
        return socket_sendto(f->socket, buf, len, addr, addrlen);
    else if (f->socket->type == SOCK_STREAM)
        return socket_write(f->socket, buf, len);
    else
        return -EINVAL;
}

// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
long sys_connect(void)
{
    struct file *f;
    struct sockaddr *addr;
    int addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argptr(2, (void **)&addrlen, sizeof(int)) <0)
        return -EFAULT;

    return socket_connect(f->socket, addr, addrlen);
}

// int listen(int sockfd, int backlog);
long sys_listen(void)
{
    struct file *f;
    int backlog;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argint(1, &backlog) < 0)
        return -EINVAL;

    return socket_listen(f->socket, backlog);
}

// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
long sys_accept(void)
{
    struct file *f;
    struct sockaddr *addr;
    int *addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (f->type != FD_SOCKET)
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argptr(2, (void **)&addrlen, sizeof(int)) <0)
        return -EFAULT;

    return socket_accept(f->socket, addr, addrlen);
}

long sys_dns(void)
{
    //ip_addr_t addr;
    //dns_resolve("www.google.com", &addr);
    ntp_get_time();
    return 0;
}
