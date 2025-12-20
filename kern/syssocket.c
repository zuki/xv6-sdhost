// From https://github.com/pandax381/xv6-riscv-net
#include <types.h>
#include <arm.h>
#include <syscall.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <proc.h>
#include <console.h>
#include <net/socket.h>
#include <linux/errno.h>
#include <net/dns.h>
#include <vfs.h>
#include <filedesc.h>
#include <fs/vfile.h>

// socket(int domain, int type, int protocol);
// TODO: アクセス権限チェック
long sys_socket(void)
{
    int domain, type, protocol;
    int fd;
    long error;
    struct vfile *file;
    struct proc *p = thisproc();

    if (argint(0, &domain) < 0 || argint(1, &type) < 0
     || argint(2, &protocol) < 0)
        return -EINVAL;
    trace("domain: %d, type: %d, protocol: %d", domain, type, protocol);

    if ((fd = find_unused_fd(p->fd_table, 0)) < 0)
        return fd;

    if ((error = socket_alloc(domain, type, protocol, p->uid, &file)) < 0)
        return error;

    set_fd(p->fd_table, fd, file);

    return fd;
}

// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
long sys_bind(void)
{
    struct vfile *f;
    struct sockaddr *addr;
    int addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;

    if (!S_ISSOCK(f->vnode->mode))
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argint(2, &addrlen) < 0)
        return -EINVAL;

    return socket_bind(SOCKET(f->vnode), addr, addrlen);
}

// ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
long sys_recvfrom(void)
{
    struct vfile *f;
    char *buf;
    int flags;
    size_t len;
    struct sockaddr *addr;
    int *addrlen;
    struct socket *sock;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (!S_ISSOCK(f->vnode->mode))
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

    sock = SOCKET(f->vnode);
    if (sock->type == SOCK_DGRAM)
        return socket_recvfrom(sock, buf, len, addr, addrlen);
    else if (sock->type == SOCK_STREAM)
        return socket_read(f, buf, len);
    else
        return -EINVAL;
}

// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
long sys_sendto(void)
{
    struct vfile *f;
    char *buf;
    int flags;
    size_t len;
    struct sockaddr *addr;
    int addrlen;
    struct socket *sock;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (!S_ISSOCK(f->vnode->mode))
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

    sock = SOCKET(f->vnode);
    if (sock->type == SOCK_DGRAM)
        return socket_sendto(sock, buf, len, addr, addrlen);
    else if (sock->type == SOCK_STREAM)
        return socket_write(f, buf, len);
    else
        return -EINVAL;
}

// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
long sys_connect(void)
{
    struct vfile *f;
    struct sockaddr *addr;
    int addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (!S_ISSOCK(f->vnode->mode))
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argptr(2, (void **)&addrlen, sizeof(int)) <0)
        return -EFAULT;

    return socket_connect(SOCKET(f->vnode), addr, addrlen);
}

// int listen(int sockfd, int backlog);
long sys_listen(void)
{
    struct vfile *f;
    int backlog;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (!S_ISSOCK(f->vnode->mode))
        return -ENOTSOCK;

    if (argint(1, &backlog) < 0)
        return -EINVAL;

    return socket_listen(SOCKET(f->vnode), backlog);
}

// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
long sys_accept(void)
{
    struct vfile *f;
    struct sockaddr *addr;
    int *addrlen;

    if (argfd(0, 0, &f) < 0)
        return -EBADF;
    if (!S_ISSOCK(f->vnode->mode))
        return -ENOTSOCK;

    if (argptr(1, (void **)&addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    if (argptr(2, (void **)&addrlen, sizeof(int)) <0)
        return -EFAULT;

    return socket_accept(SOCKET(f->vnode), addr, addrlen);
}

long sys_dns(void)
{
    #include <net/ntp.h>

    //ip_addr_t addr;
    //dns_resolve("www.google.com", &addr);
    ntp_get_time();
    return 0;
}
