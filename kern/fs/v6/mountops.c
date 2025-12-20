#include <types.h>
#include <vfs.h>
#include <fs/v6/param.h>
#include <fs/v6/file.h>
#include <fs/v6/fs.h>
#include <linux/errno.h>

struct mount_ops v6_mount_ops = {
    "v6",
    v6_init,
    v6_mount,
    v6_unmount,
    v6_sync,
};

int v6_init(void)
{
    v6_iinit(DEVV6);
    return 0;
}

int v6_mount(struct mount *mp, device_t dev, struct vnode *parent)
{
    mp->ops = &v6_mount_ops;
    mp->dev = dev;
    mp->super = V6SBP;
    struct v6_inode *ip = v6_iget(mp, V6_ROOTINO);
    if (ip == NULL) {
        return -ENOMEM;
    }
    mp->root_node = &ip->vnode;

    return 0;
}

int v6_unmount(struct mount *mp)
{
    vfs_release_vnode(mp->root_node);
    mp->root_node = NULL;
    return 0;
}

int v6_sync(struct mount *mp)
{
    sync_v6_inodes();
    sync_bufcache();
}
