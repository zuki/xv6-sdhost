
#include <types.h>
#include <string.h>
#include <proc.h>
#include <fs/procfs/data.h>
#include <fs/procfs/procfs.h>
#include <console.h>

static inline char get_proc_state(struct proc *proc);
static inline size_t get_proc_size(struct proc *proc);


int get_data_cmdline(struct proc *proc, char *buffer, int max)
{
    return snprintf(buffer, max, "%s\n", proc->name);

#if 0
    int i = 0;
    for (const char *arg = proc->name; *arg; arg++) {
        strncpy(&buffer[i], *arg, max - i);
        i += strlen(*arg) + 1;
        buffer[i - 1] = ' ';
        if (i > max)
            break;
    }
    buffer[i - 1] = '\0';
    return i;
#endif
}


int get_data_stat(struct proc *proc, char *buffer, int max)
{
    return snprintf(buffer, max,
        "%d %s %c %d %d %d %d %d %d\n",
        proc->pid,
        proc->name,
        get_proc_state(proc),
        proc->parent ? proc->parent->pid : -1,
        0,
        0,
        0,
        0,
        proc->sz
    );
}

int get_data_statm(struct proc *proc, char *buffer, int max)
{
    return snprintf(buffer, max,
        "%d %x %x %x %x %x %x %x\n",
        proc->sz,
        proc->base,
        proc->stksz,
        0,
        0,
        proc->chan,
        proc->tf,
        proc->context
    );
}


static inline char get_proc_state(struct proc *proc)
{
    //{ UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
    switch (proc->state) {
        case UNUSED:
            return 'U';
        case EMBRYO:
            return 'E';
        case SLEEPING:
            return 'S';
        case RUNNABLE:
            return 'N';
        case RUNNING:
            return 'R';
        case ZOMBIE:
            return 'Z';
        default:
            return '?';
    }
}

static inline size_t get_proc_size(struct proc *proc)
{
#if 0
    size_t size = 0;
    for (char i = 0; i < NUM_SEGMENTS; i++)
        size += proc->map.segments[i].length;
    return size;
#endif
    return proc->sz;
}

int get_data_mounts(struct proc *proc, char *buffer, int max)
{
    size_t i = 0;
    char name[100];
    struct mount *mp;
    struct mount_iter iter;
    memset(name, 0, 100);

    vfs_mount_iter_start(&iter);

    while ((mp = vfs_mount_iter_next(&iter))) {
        trace("mp fs: %s, mount_node: %d", mp->ops->fstype, mp->mount_node ? mp->mount_node->ino : -1);
        vfs_reverse_lookup(mp->mount_node, name, 100, thisproc()->uid);
        trace("name: %s", name);
        i += snprintf(&buffer[i], max - i, "%s %x %s %s\n", name, mp->dev, mp->ops->fstype, (mp->bits & VFS_MBF_READ_ONLY) ? "ro" : "rw");
    }
    trace("buffer: %s, i: %d", buffer, i);
    return i;
}

int get_dir_entry(struct vfile *file, char *buffer, int max)
{
    int err;
    struct dirent de;
    memset(&de, 0, DESIZE);
    //extern struct procfs_dir_entry *root_files;


    if ((err = procfs_readdir(file, &de)) < 0) {
        error("err: %d", err);
        return err;
    } else if (err == 0) {
        trace("readdir is fine");
        return 0;
    }

    trace("return code: 0x%x, file->offset: 0x%x", err, file->offset);
    trace("dirent ino: %d, type: %d, name: %s", de.ino, de.type, de.name);
    //hexdump(&de, DESIZE, "dirent");
    memmove(buffer, &de, DESIZE);
    return DESIZE;
}
