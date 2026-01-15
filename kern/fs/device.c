
#include <types.h>
#include <driver.h>
#include <linux/errno.h>
#include <vfs.h>

#include <fs/nop.h>
#include <fs/device.h>

struct vfile_ops device_vfile_ops = {
	device_ops_open,
	device_ops_close,
	device_ops_read,
	device_ops_write,
	device_ops_ioctl,
	device_ops_poll,
	device_ops_seek,
	nop_readdir,
    nop_getdents,
    nop_writeback,
    nop_chown,
};

int device_ops_open(struct vfile *file, int flags)
{
	return dev_open(file->vnode->rdev, flags);
}

int device_ops_close(struct vfile *file)
{
	return dev_close(file->vnode->rdev);
}

int device_ops_read(struct vfile *file, char *buffer, size_t size)
{
	int nbytes;

	nbytes = dev_read(file->vnode->rdev, buffer, file->offset, size);
	if (nbytes < 0)
		return nbytes;
	file->offset += nbytes;
	return nbytes;
}

int device_ops_write(struct vfile *file, const char *buffer, size_t size)
{
	int nbytes;

	nbytes = dev_write(file->vnode->rdev, buffer, file->offset, size);
	if (nbytes < 0)
		return nbytes;
	file->offset += nbytes;
	return nbytes;
}

int device_ops_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid)
{
	return dev_ioctl(file->vnode->rdev, request, argp, uid);
}

int device_ops_poll(struct vfile *file, int events)
{
	return dev_poll(file->vnode->rdev, events);
}

off_t device_ops_seek(struct vfile *file, off_t offset, int whence)
{
	int pos;

	pos = dev_seek(file->vnode->rdev, offset, whence, file->offset);
	if (pos < 0)
		return pos;
	file->offset = pos;
	return pos;
}
