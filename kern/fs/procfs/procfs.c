#include <types.h>
#include <vfs.h>
#include <driver.h>
#include <string.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <proc.h>
#include <fs/procfs/data.h>
#include <fs/procfs/procfs.h>
#include <fs/nop.h>
#include <console.h>
#include <clock.h>

#define PROCFS_POSITION(x)    ((struct procfs_position *) &(x))

struct vfile_ops procfs_vfile_ops = {
    procfs_open,
    procfs_close,
    procfs_read,
    procfs_write,
    procfs_ioctl,
    nop_poll,
    procfs_seek,
    procfs_readdir,
    procfs_getdents,
};

struct vnode_ops procfs_vnode_ops = {
    &procfs_vfile_ops,
    nop_create,
    nop_mknod,
    procfs_lookup,
    nop_link,
    nop_symlink,
    nop_unlink,
    nop_rename,
    nop_truncate,
    nop_update,
    procfs_release,
    nop_rmdir,
};

struct mount_ops procfs_mount_ops = {
    "procfs",
    procfs_init,
    procfs_mount,
    procfs_unmount,
    nop_sync,
};

struct procfs_dir_entry root_files[] = {
    { PFN_ROOTDIR,  ".",        NULL, get_dir_entry },
    { PFN_ROOTDIR,  "..",       NULL, NULL },
    { PFN_MOUNTS,   "mounts",   get_data_mounts, NULL },
    { 0, NULL, NULL, NULL },
};

struct procfs_dir_entry proc_files[] = {
    { PFN_PROCDIR,  ".",        NULL, get_dir_entry },
    { PFN_ROOTDIR,  "..",       NULL, get_dir_entry },
    { PFN_CMDLINE,  "cmdline",  get_data_cmdline, NULL },
    { PFN_STAT,     "stat",     get_data_stat, NULL },
    { PFN_STATM,    "statm",    get_data_statm, NULL },
    { 0, NULL, NULL, NULL },
};

#define MAX_VNODES    6
#define MAX_BUFFER    256

static struct procfs_vnode vnode_table[MAX_VNODES];

static procfs_filenum_t _find_filenum_by_name(struct procfs_dir_entry *entries, const char *filename);
static struct procfs_dir_entry *_get_entry_by_num(struct procfs_dir_entry *entries, procfs_filenum_t filenum);
static struct vnode *_find_vnode(pid_t pid, procfs_filenum_t filenum, mode_t mode, struct mount *mp);
static struct vnode *_alloc_vnode(pid_t pid, procfs_filenum_t filenum, mode_t mode, struct mount *mp);

int procfs_init()
{
    for (int i = 0; i < MAX_VNODES; i++)
        vnode_table[i].vn.refcount = 0;
    trace("procfs_init ok");
    return 0;
}

int procfs_mount(struct mount *mp, device_t dev, struct vnode *parent)
{
    mp->root_node = _alloc_vnode(0, 0, S_IFDIR | 0755, mp);
    return 0;
}

int procfs_unmount(struct mount *mp)
{
    vfs_release_vnode(mp->root_node);
    return 0;
}

int procfs_lookup(struct vnode *vnode, const char *filename, struct vnode **result)
{
    pid_t pid = 0;
    procfs_filenum_t filenum;
    struct proc *proc;
    trace("vnode: 0x%llx (%d), filename: %s, result: 0x%llx", vnode, vnode->ino, filename, result);

    /* 1. PFN_ROOTDIR */
    if (PROCFS_DATA(vnode).filenum == PFN_ROOTDIR) {
        /* ファイル名がproc番号 */
        if (*filename >= '0' && *filename <= '9') {
            /* ファイル名からpidを作成 */
            pid = strtol(filename, NULL, 10);
            filenum = PFN_PROCDIR;
            /* pidを持つstruct procを取り出す */
            proc = get_proc(pid);
            if (!proc) {
                error("[0] pid: %d is not exists", pid);
                return -ENOENT;
            } else {
                trace("[0] %s is proc with pid: %d, name: %s, filenum: %d", filename, proc->pid, proc->name, filenum);
            }
        /* ファイル名が普通のファイル名 */
        } else {
            filenum = _find_filenum_by_name(root_files, filename);
            if (filenum < 0) {
                error("[0] filename: %s is not exists in root_files", filename);
                return -ENOENT;
            } else {
                trace("[0] %s is filenum: %d", filename, filenum);
            }
        }
    /* PFN_PROCDIR */
    } else if (PROCFS_DATA(vnode).filenum == PFN_PROCDIR) {
        pid = PROCFS_DATA(vnode).pid;
        filenum = _find_filenum_by_name(proc_files, filename);
        if (filenum < 0) {
            error("[1] filename: %s is not exists in proc_files", filename);
            return -ENOENT;
        } else {
            trace("[1] %s is proc with pid: %d, filenum: %d", filename, pid, filenum);
        }
    } else {
        error("[2] fileenum: %d is not directry", PROCFS_DATA(vnode).filenum);
        return -ENOTDIR;
    }

    if (*result)
        vfs_release_vnode(*result);
    *result = _find_vnode(pid, filenum, S_IFDIR | 0755, vnode->mp);
    return 0;
}

int procfs_release(struct vnode *vnode)
{
    return 0;
}

int procfs_open(struct vfile *file, int flags)
{
    trace("file: 0x%x, f->vnode->ino: %d", file, file->vnode->ino);
    if (PROCFS_DATA(file->vnode).filenum == PFN_ROOTDIR) {
        PROCFS_POSITION(file->offset)->slot = 0;
        proc_iter_start(&PROCFS_POSITION(file->offset)->iter);
    } else {
        file->offset = 0;
    }
    return 0;
}

int procfs_close(struct vfile *file)
{
    return 0;
}

int procfs_read(struct vfile *file, char *buf, size_t nbytes)
{
    int limit = 0;
    struct proc *proc;
    char buffer[MAX_BUFFER];
    struct procfs_dir_entry *entry;
#if 0
    trace("file: pid: %d, ino: %d, nbytes: 0x%x", PROCFS_DATA(file->vnode).pid, file->vnode->ino, nbytes);
    trace("PROCFS_DATA(file->vnode): pid: %d, filenum: %d", PROCFS_DATA(file->vnode).pid, PROCFS_DATA(file->vnode).filenum);
#endif

    proc = get_proc(PROCFS_DATA(file->vnode).pid);
    if (PROCFS_DATA(file->vnode).pid != 0 && !proc) {
        error("pid: %d and proc is null", PROCFS_DATA(file->vnode).pid)
        return -ENOENT;
    }

    entry = _get_entry_by_num(proc_files, PROCFS_DATA(file->vnode).filenum);
    if (entry) {
        trace("entry from proc_files: filenum: %d", PROCFS_DATA(file->vnode).filenum);
    }

    if (!entry) {
        entry = _get_entry_by_num(root_files, PROCFS_DATA(file->vnode).filenum);
        if (entry) {
            trace("entry from root_files: filenum: %d", PROCFS_DATA(file->vnode).filenum);
        }
    }

    if (entry && entry->func) {
        trace("exec func: 0x%llx with proc->pid: %d", entry->func, proc->pid);
        limit = entry->func(proc, buffer, MAX_BUFFER);
        trace("%s", buffer);
    } else if (entry && entry->func2) {
        trace("exec func2: 0x%llx with file->vnode: %d", entry->func2, file->vnode->ino);
        limit = entry->func2(file, buf, nbytes);
        trace("file->offset: 0x%x", file->offset);
        return limit;
    }

    trace("[0] offset: 0x%x, nbytes: 0x%x, limit: 0x%x", file->offset, nbytes, limit);
    if (file->offset + nbytes >= limit)
        nbytes = limit - file->offset;
    if (nbytes)
        strncpy(buf, &buffer[file->offset], nbytes);
    file->offset += nbytes;
    trace("[1] offset: 0x%x, nbytes: 0x%x, limit: 0x%x", file->offset, nbytes, limit);
    return nbytes;
}

int procfs_write(struct vfile *file, const char *buf, size_t nbytes)
{
    return 0;
}

int procfs_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid)
{
    return -1;
}

off_t procfs_seek(struct vfile *file, off_t position, int whence)
{
    return -1;
}

#define PROCFS_MAX_PID        0x10000

int procfs_readdir(struct vfile *file, struct dirent *dir)
{
    int i;
    short slot;
    struct proc *proc;

    trace("file->vnode: ino: %d, mode: 0x%x, offset: %d", file->vnode->ino, file->vnode->mode, file->offset);

    if (!S_ISDIR(file->vnode->mode)) {
        trace("file mode: 0x%x is not dir", file->vnode->mode);
        return -ENOTDIR;
    }

    trace("PROCFS_DATA(file->vnode).filenum: %d", PROCFS_DATA(file->vnode).filenum);

    if (PROCFS_DATA(file->vnode).filenum == PFN_ROOTDIR) {
        if (PROCFS_POSITION(file->offset)->slot == 0 && (proc = proc_iter_next(&PROCFS_POSITION(file->offset)->iter))) {            // 実行中のpidを表示
            //dir->ino = file->offset;
            dir->ino = PFN_PROCDIR;
            snprintf(dir->name, VFS_FILENAME_MAX, "%d", proc->pid);
            dir->name[VFS_FILENAME_MAX - 1] = '\0';
            dir->type = 1;          // TODO: typeは要検討
            trace("(1-1) dir: ino: 0x%x, name: %s", dir->ino, dir->name);
        } else {                            // root_filesを表示
            slot = ++PROCFS_POSITION(file->offset)->slot;

            if (!root_files[slot - 1].filename) {
                trace("root_files is fin");
                return 0;
            }

            dir->ino = file->offset;
            strncpy(dir->name, root_files[slot - 1].filename, VFS_FILENAME_MAX);
            dir->name[VFS_FILENAME_MAX - 1] = '\0';
            dir->type = 1;          // TODO
            trace("(1-2) dir->name: %s", dir->name);
        }
    } else if (PROCFS_DATA(file->vnode).filenum == PFN_PROCDIR) {
        if (!proc_files[file->offset].filename) {
            trace("prof_files is fin");
            return 0;
        }
        trace("(2) source: pid: %d, num: %d, name: '%s', offset: %d", PROCFS_DATA(file->vnode).pid, proc_files[file->offset].filenum, proc_files[file->offset].filename, file->offset);
        dir->ino = (PROCFS_DATA(file->vnode).pid << 8) | proc_files[file->offset].filenum;
        strncpy(dir->name, proc_files[file->offset].filename, VFS_FILENAME_MAX);
        dir->name[VFS_FILENAME_MAX - 1] = '\0';
        dir->type = 1;
        file->offset += 1;
        trace("(2) dir->name: '%s', dir->ino: 0x%x, offset: %d", dir->name, dir->ino, file->offset);
    } else {
        error("PROCFS_DATA(file->vnode).filenum is not dir");
        return -ENOTDIR;
    }
    trace("dir ino: %d, type: %d, name: %s", dir->ino, dir->type, dir->name);

    return 1;
}

static int procfs_set_dirent64(struct dirent64 *dir, int ino, off64_t offset, uint8_t type, char *name)
{
    int namelen, reclen;

    namelen = MIN(strlen(name), VFS_FILENAME_MAX) + 1;
    reclen = (size_t)(&((struct dirent64 *)0)->d_name) + namelen;
    reclen = (reclen + 0x7) & ~0x7;
    dir->d_ino = ino;
    dir->d_off = offset;
    dir->d_reclen = reclen;
    dir->d_type = type;
    memmove(dir->d_name, name, namelen);
    return reclen;
}
int procfs_getdents(struct vfile *file, void *buffer, size_t size)
{
    int i, slot, ino;
    int reclen, tlen = 0;
    struct proc *proc;
    struct dirent64 dir;
    off_t offset = file->offset;
    char name[VFS_FILENAME_MAX];

    trace("file->vnode: ino: %d, mode: 0x%x, offset: %d", file->vnode->ino, file->vnode->mode, file->offset);

    trace("PROCFS_DATA(file->vnode).filenum: %d", PROCFS_DATA(file->vnode).filenum);

    if (PROCFS_DATA(file->vnode).filenum == PFN_ROOTDIR) {
        if (PROCFS_POSITION(file->offset)->slot == 0 && (proc = proc_iter_next(&PROCFS_POSITION(file->offset)->iter))) {            // 実行中のpidを表示
            ino = file->offset;
            //dir.d_ino = PFN_PROCDIR;
            snprintf(name, VFS_FILENAME_MAX, "%d", proc->pid);
            reclen = procfs_set_dirent64(&dir, ino, offset, IFTODT(file->vnode->mode), name);

            if ((tlen + reclen) > size) {
                trace("break; tlen: %d, reclen: %d, size: %d", tlen, reclen, size);
                return tlen;
            }
            memmove(buffer + tlen, (char *)&dir, reclen);
            tlen += reclen;
            offset = file->offset;
            trace("(1-1) dir: ino: 0x%x, name: %s", dir.d_ino, dir.d_name);
        } else {                            // root_filesを表示
            slot = ++PROCFS_POSITION(file->offset)->slot;

            if (!root_files[slot - 1].filename) {
                trace("root_files is fin");
                return 0;
            }
            trace("(1-2) source: slot: %d, name: '%s', offset: %d", slot - 1, root_files[slot - 1].filename, offset);
            ino = file->offset;
            strncpy(name, root_files[slot - 1].filename, VFS_FILENAME_MAX);
            name[VFS_FILENAME_MAX - 1] = '\0';
            reclen = procfs_set_dirent64(&dir, ino, offset, IFTODT(file->vnode->mode), name);

            if ((tlen + reclen) > size) {
                trace("break; tlen: %d, reclen: %d, size: %d", tlen, reclen, size);
                return tlen;
            }
            memmove(buffer + tlen, (char *)&dir, reclen);
            tlen += reclen;
            offset = file->offset;
            trace("(1-2) dir.d_name: %s", dir.d_name);
        }
    } else if (PROCFS_DATA(file->vnode).filenum == PFN_PROCDIR) {
        if (!proc_files[file->offset].filename) {
            trace("prof_files is fin");
            return 0;
        }
        ino = (PROCFS_DATA(file->vnode).pid << 8) | proc_files[file->offset].filenum;
        trace("(2) source pid: %d, num: %d, ino: 0x%x, name: '%s', offset: %d", PROCFS_DATA(file->vnode).pid, proc_files[file->offset].filenum, ino, proc_files[file->offset].filename, file->offset);
        strncpy(name, proc_files[file->offset].filename, VFS_FILENAME_MAX);
        name[VFS_FILENAME_MAX - 1] = '\0';

        reclen = procfs_set_dirent64(&dir, ino, offset, IFTODT(file->vnode->mode), name);
        if ((tlen + reclen) > size) {
            trace("break; tlen: %d, reclen: %d, size: %d", tlen, reclen, size);
            return tlen;
        }
        memmove(buffer + tlen, (char *)&dir, reclen);
        tlen += reclen;
        offset = ++file->offset;
        trace("(2) dir.d_name: %s, dir.d_ino: 0x%x, offset: %d", dir.d_name, dir.d_ino, file->offset);
    } else {
        error("PROCFS_DATA(file->vnode).filenum is not dir");
        return -ENOTDIR;
    }
    trace("dir ino: 0x%x, type: %d, name: %s", dir.d_ino, dir.d_type, dir.d_name);

    return 1;
}


static procfs_filenum_t _find_filenum_by_name(struct procfs_dir_entry *entries, const char *filename)
{
    procfs_filenum_t filenum;

    for (int i = 0; entries[i].filename; i++) {
        if (!strcmp(filename, entries[i].filename)) {
            filenum = entries[i].filenum;
            trace("fn %s == ent[%d] %s and return %d", filename, i, entries[i].filename, filenum);
            return filenum;
        } else {
            trace("fn %s != ent[%d] %s", filename, i, entries[i].filename);
        }
    }
    return ENOENT;
}

static struct procfs_dir_entry *_get_entry_by_num(struct procfs_dir_entry *entries, procfs_filenum_t filenum)
{
    for (int i = 0; entries[i].filename; i++) {
        if (entries[i].filenum == filenum)
            return &entries[i];
    }
    return NULL;
}

static struct vnode *_find_vnode(pid_t pid, procfs_filenum_t filenum, mode_t mode, struct mount *mp)
{
    trace("pid: %d, filenum: %d, mode: 0x%x, mp->root_node: 0x%x", pid, filenum, mode, mp->root_node->ino);
    for (int i = 0; i < MAX_VNODES; i++) {
        trace("vnode_table[%d] vn.refcount: %d, pid: %d, filenum: %d", i, vnode_table[i].vn.refcount, PROCFS_DATA(&vnode_table[i]).pid, PROCFS_DATA(&vnode_table[i]).filenum);
        if (vnode_table[i].vn.refcount > 0 && PROCFS_DATA(&vnode_table[i]).pid == pid && PROCFS_DATA(&vnode_table[i]).filenum == filenum) {
            trace("[%d] CLONE: vnode->ino: %d, mode: 0x%x, filenum: %d", i, vnode_table[i].vn.ino, vnode_table[i].vn.mode, PROCFS_DATA(&vnode_table[i]).filenum);
            trace("return vfs_clone_vnode");
            return vfs_clone_vnode(&vnode_table[i].vn);
        }
    }
    trace("call _alloc_vnode");
    return _alloc_vnode(pid, filenum, mode, mp);
}

static struct vnode *_alloc_vnode(pid_t pid, procfs_filenum_t filenum, mode_t mode, struct mount *mp)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    trace("pid: %d, filenum: %d, mode: 0x%x, mp->root_node: 0x%x", pid, filenum, mode, mp->root_node->ino);
    for (int i = 0; i < MAX_VNODES; i++) {
        if (vnode_table[i].vn.refcount <= 0) {
            vfs_init_vnode(&vnode_table[i].vn, &procfs_vnode_ops, mp, mode, 1, 0, 0, 0, filenum, 0, &now, &now, &now);
            PROCFS_DATA(&vnode_table[i]).pid = pid;
            PROCFS_DATA(&vnode_table[i]).filenum = filenum;
            trace("[%d] ALLOC: vnode->ino: %d, mode: 0x%x, filenum: %d", i, vnode_table[i].vn.ino, vnode_table[i].vn.mode, PROCFS_DATA(&vnode_table[i]).filenum);
            return &vnode_table[i].vn;
        }
    }
    trace("return NULL");
    return NULL;
}

struct procfs_dir_entry *get_dir_entry_by_index(const char *dir, int index)
{
    if (!strncmp(dir, "root", 4)) {
        if (index >= 0 && index < (ARRAY_SIZE(root_files) - 1))
            return &root_files[index];
    } else if (!strncmp(dir, "proc", 4)) {
        if (index >= 0 && index < (ARRAY_SIZE(proc_files) - 1))
            return &proc_files[index];
    }

    return NULL;
}
