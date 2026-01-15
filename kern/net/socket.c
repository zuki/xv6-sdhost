#include <types.h>
#include <config.h>
#include <net/socket.h>
#include <net/sockio.h>
#include <net/if.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/platform.h>
#include <sleeplock.h>
#include <vfs.h>
#include <fs/vfile.h>
#include <fs/nop.h>
#include <filedesc.h>
#include <mm.h>
#include <string.h>
#include <linux/errno.h>
#include <console.h>

struct vfile_ops sock_vfile_ops = {
    nop_open,
    socket_close,
    socket_read,
    socket_write,
    socket_ioctl,
    nop_poll,
    nop_seek,
    nop_readdir,
    nop_getdents,
    nop_writeback,
    nop_chown,
};

struct vnode_ops sock_vnode_ops = {
    &sock_vfile_ops,
    nop_create,
	nop_mknod,
	nop_lookup,
	nop_link,
    nop_symlink,
	nop_unlink,
	nop_rename,
	nop_truncate,
	nop_update,
	socket_release_vnode,
    nop_rmdir,
};

int socket_alloc(int domain, int type, int protocol, uid_t uid, struct vfile **file)
{
    struct vfile *f;
    struct socket *s;
    struct sock_vnode *vnode;

    if (domain != AF_INET || (protocol != 0 && protocol != IP_PROTOCOL_ICMP && protocol != IP_PROTOCOL_TCP && protocol != IP_PROTOCOL_UDP)) {
        error("bad domain: %d or protocol: %d", domain, protocol);
        return -EINVAL;
    }

    if ((vnode = kmalloc(sizeof(struct sock_vnode))) == NULL)
        return -ENOMEM;
    vfs_init_vnode(&vnode->vn, &sock_vnode_ops, NULL, S_IFSOCK | 0600, 1, uid, 0, 0, 0, 0, 0, 0, 0);

    if ((f = alloc_file(&vnode->vn, 0)) == NULL) {
        kmfree(vnode);
        return -EMFILE;
    }

    s = SOCKET(vnode);

    s->type = type;
    switch(type) {
    case SOCK_DGRAM:
        s->desc = udp_open();
        break;
    case SOCK_STREAM:
        s->desc = tcp_open();
        break;
    default:
        error("wrong type: %d", type);
        kmfree(vnode);
        return -EAFNOSUPPORT;
    }

    if (*file)
        *file = f;
    trace("socket: %d, desc: %d", fd, s->desc);
    return 0;
}

int socket_release_vnode(struct vnode *vnode)
{
    kmfree(vnode);
    return 0;
}

int socket_close(struct vfile *file)
{
    struct socket *s = SOCKET(file->vnode);

    switch (s->type) {
    case SOCK_DGRAM:
        udp_close(s->desc);
        break;
    case SOCK_STREAM:
        tcp_close(s->desc);
        break;
    default:
        return -1;
    }
    //kmfree(vnode);
    return 0;
}

int socket_bind(struct socket *s, struct sockaddr *addr, int addrlen)
{
    struct ip_endpoint local;

    local.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    local.port = ((struct sockaddr_in *)addr)->sin_port;
    switch (s->type) {
    case SOCK_DGRAM:
        return udp_bind(s->desc, &local);
    case SOCK_STREAM:
        return tcp_bind(s->desc, &local);
    default:
        return -EINVAL;
    }
}

int socket_recvfrom(struct socket *s, char *buf, int n, struct sockaddr *addr, int *addrlen)
{
    struct ip_endpoint foreign;
    int ret;

    if (s->type != SOCK_DGRAM) {
        return -EINVAL;
    }
    ret = udp_recvfrom(s->desc, (uint8_t *)buf, n, &foreign);
    if (addr) {
        ((struct sockaddr_in *)addr)->sin_family = AF_INET;
        ((struct sockaddr_in *)addr)->sin_addr.s_addr = foreign.addr;
        ((struct sockaddr_in *)addr)->sin_port = foreign.port;
        if (addrlen)
            *addrlen = sizeof(struct sockaddr_in);
    }

    return ret;
}

int socket_sendto(struct socket *s, char *buf, int n, struct sockaddr *addr, int addrlen)
{
    struct ip_endpoint foreign;

    if (s->type != SOCK_DGRAM) {
        return -EINVAL;
    }
    foreign.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    foreign.port = ((struct sockaddr_in *)addr)->sin_port;
    return udp_sendto(s->desc, (uint8_t *)buf, n, &foreign);
}

int socket_connect(struct socket *s, struct sockaddr *addr, int addrlen)
{
    struct ip_endpoint foreign;

    if (s->type != SOCK_STREAM) {
      return -EINVAL;
    }
    foreign.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    foreign.port = ((struct sockaddr_in *)addr)->sin_port;
    return tcp_connect(s->desc, &foreign);
}

int socket_listen(struct socket *s, int backlog)
{
    if (s->type != SOCK_STREAM) {
        return -EOPNOTSUPP;
    }
    return tcp_listen(s->desc, backlog);
}

int socket_accept(struct socket *s, struct sockaddr *addr, int *addrlen)
{
    int adesc, fd;
    struct vfile *file;
    struct socket *as;
    struct ip_endpoint foreign;
    struct proc *p = thisproc();
    struct sock_vnode *vnode;

    if (s->type != SOCK_STREAM) {
        return -EOPNOTSUPP;
    }

    if ((fd = find_unused_fd(p->fd_table, 0)) < 0)
        return fd;

    if ((vnode = kmalloc(sizeof(struct sock_vnode))) == NULL)
        return -ENOMEM;
    vfs_init_vnode(&vnode->vn, &sock_vnode_ops, NULL, S_IFSOCK | 0600, 1, p->uid, 0, 0, 0, 0, 0, 0, 0);

    if ((file = alloc_file(&vnode->vn, 0)) == NULL) {
        kmfree(vnode);
        return -EMFILE;
    }

    set_fd(p->fd_table, fd, file);

    adesc = tcp_accept(SOCKET(vnode)->desc, &foreign);
    if (adesc == -1) {
        unset_fd(p->fd_table, fd);
        kmfree(vnode);
        return -EPROTO;
    }
    ((struct sockaddr_in *)addr)->sin_family = AF_INET;
    ((struct sockaddr_in *)addr)->sin_addr.s_addr = foreign.addr;
    ((struct sockaddr_in *)addr)->sin_port = foreign.port;
    as->type = s->type;
    as->desc = adesc;

    if (addrlen)
        *addrlen = sizeof(struct sockaddr_in);
    return fd;
}

int socket_read(struct vfile *file, char *buf, size_t n)
{
    struct socket *s = SOCKET(file->vnode);

    if (s->type != SOCK_STREAM) {
        return -EINVAL;
    }
    return tcp_receive(s->desc, (uint8_t *)buf, n);
}

int socket_write(struct vfile *file, char *buf, size_t n)
{
    struct socket *s = SOCKET(file->vnode);

    if (s->type != SOCK_STREAM) {
        return -EINVAL;
    }
    return tcp_send(s->desc, (uint8_t *)buf, n);
}

int socket_ioctl(struct vfile *file, uint32_t req, void *arg, uid_t uid)
{
    struct ifreq *ifreq;
    struct net_device *dev;
    struct ip_iface *iface;
    char addr[IP_ADDR_STR_LEN];

    ifreq = (struct ifreq *)arg;
    switch ((unsigned int)req) {
    // インターフェースの interface index を取得し、 ifr_ifindex に入れて返す
    case SIOCGIFINDEX:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        ifreq->ifr_ifindex = dev->index;
        break;
    // ifr_ifindex を受け取り、インターフェースの名前を ifr_name に入れて返す。
    case SIOCGIFNAME:
        dev = net_device_by_index(ifreq->ifr_ifindex);
        if (!dev) {
            return -1;
        }
        strncpy(ifreq->ifr_name, dev->name, sizeof(ifreq->ifr_name));
        break;
    case SIOCSIFNAME:
        // TODO
        break;
    case SIOCGIFHWADDR:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        // TODO: HW type check
        memcpy(ifreq->ifr_hwaddr.sa_data, dev->addr, dev->alen);
        break;
    case SIOCSIFHWADDR:
        // TODO
        break;
    // デバイスの active フラグワードを取得する
    case SIOCGIFFLAGS:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        ifreq->ifr_flags = dev->flags;
        break;
    // デバイスの active フラグワードを設定する
    case SIOCSIFFLAGS:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        if ((dev->flags & IFF_UP) != (ifreq->ifr_flags & IFF_UP)) {
            if (ifreq->ifr_flags & IFF_UP) {
                net_device_open(dev);
            } else {
                net_device_close(dev);
            }
        }
        break;
    // ifr_addr を用いてデバイスのアドレスを取得する
    case SIOCGIFADDR:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        switch (ifreq->ifr_addr.sa_family) {
        case AF_INET:
            iface = (struct ip_iface *)net_device_get_iface(dev, NET_IFACE_FAMILY_IP);
            if (!iface) {
                return -1;
            }
            ((struct sockaddr_in *)&ifreq->ifr_addr)->sin_addr.s_addr = iface->unicast;
            break;
        default:
            return -1;
        }
        break;
    // ifr_addr を用いてデバイスのアドレスを設定する
    case SIOCSIFADDR:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            error("failed SIOCSIFADDR: %s", ifreq->ifr_name);
            return -1;
        }
        switch (ifreq->ifr_addr.sa_family) {
        case AF_INET:
            iface = (struct ip_iface *)net_device_get_iface(dev, NET_IFACE_FAMILY_IP);
            if (iface) {
                if (ip_iface_reconfigure(iface, ((struct sockaddr_in *)&ifreq->ifr_addr)->sin_addr.s_addr, iface->netmask) == -1) {
                    return -1;
                }
            } else {
                iface = ip_iface_alloc(ip_addr_ntop(((struct sockaddr_in *)&ifreq->ifr_addr)->sin_addr.s_addr, addr, sizeof(addr)), NETMASK);
                if (!iface) {
                    return -1;
                }
                trace("register iface");
                ip_iface_register(dev, iface);
            }
            break;
        default:
            return -1;
        }
        break;
    // デバイスのネットワークマスクを ifr_netmask を用いて取得する
    case SIOCGIFNETMASK:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        switch (ifreq->ifr_addr.sa_family) {
        case AF_INET:
            iface = (struct ip_iface *)net_device_get_iface(dev, NET_IFACE_FAMILY_IP);
            if (!iface) {
                return -1;
            }
            ((struct sockaddr_in *)&ifreq->ifr_netmask)->sin_addr.s_addr = iface->netmask;
            break;
        default:
            return -1;
        }
        break;
    // デバイスのネットワークマスクを ifr_netmask を用いて設定する
    case SIOCSIFNETMASK:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        switch (ifreq->ifr_addr.sa_family) {
        case AF_INET:
            iface = (struct ip_iface *)net_device_get_iface(dev, NET_IFACE_FAMILY_IP);
            if (!iface) {
                return -1;
            }
            if (ip_iface_reconfigure(iface, iface->unicast, ((struct sockaddr_in *)&ifreq->ifr_addr)->sin_addr.s_addr) == -1) {
                return -1;
            }
            break;
        default:
            return -1;
        }
        break;
    // デバイスのブロードキャストアドレスを ifr_brdaddr を用いて取得する
    case SIOCGIFBRDADDR:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        switch (ifreq->ifr_addr.sa_family) {
        case AF_INET:
            iface = (struct ip_iface *)net_device_get_iface(dev, NET_IFACE_FAMILY_IP);
            if (!iface) {
                return -1;
            }
            ((struct sockaddr_in *)&ifreq->ifr_broadaddr)->sin_addr.s_addr = iface->broadcast;
            break;
        default:
            return -1;
        }
        break;
    // デバイスのブロードキャストアドレスを ifr_brdaddr を用いて設定する
    case SIOCSIFBRDADDR:
        // TODO
        break;
    // デバイスの MTU (Maximum Transfer Unit) を ifr_mtu を用いて取得する
    case SIOCGIFMTU:
        dev = net_device_by_name(ifreq->ifr_name);
        if (!dev) {
            return -1;
        }
        ifreq->ifr_mtu = dev->mtu;
        break;
    // デバイスの MTU (Maximum Transfer Unit) を ifr_mtu を用いて設定する
    case SIOCSIFMTU:
        break;
    default:
        return -1;
    }
    return 0;
}
