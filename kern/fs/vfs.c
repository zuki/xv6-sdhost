
#include <string.h>

#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <param.h>
#include <vfs.h>
#include <console.h>
#include <string.h>
#include <fs/vfile.h>
#include <fs/bufcache.h>
#include <fs/device.h>
#include <filedesc.h>

#define VFS_MOUNT_MAX       4

static struct mount *root_fs;
static struct mount mountpoints[VFS_MOUNT_MAX];

static struct vfile *get_file(struct vnode *vnode, int flags);

static struct mount *_find_mount_by_vnode(struct vnode *mount)
{
    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        if (mountpoints[i].mount_node == mount)
            return &mountpoints[i];
    }
    return NULL;
}

struct mount *get_rootfs(void)
{
    return root_fs;
}

int init_vfs(void)
{
    // 1. root_fsのクリア
    root_fs = NULL;
    // 2. マウントテーブルのクリア
    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        mountpoints[i].dev = 0;
    }
    // 3. バッファキャッシュの初期化
    init_bufcache();
    // 4. vfile_tableの初期化
    init_vfile();
    info("init_vfs ok");
    return 0;
}

void vfs_mount_iter_start(struct mount_iter *iter)
{
    iter->slot = 0;
}

struct mount *vfs_mount_iter_next(struct mount_iter *iter)
{
    struct mount *mp;

    do {
        if (iter->slot >= VFS_MOUNT_MAX)
            return NULL;
        mp = &mountpoints[iter->slot++];
        //if (mp->dev != 0) dump_mp(mp, "vfs_mount_iter_next");
    } while (mp->dev == 0);
    return mp;
}

//vfs_mount(NULL, "/", root_dev, &v6_mount_ops, 0, 0);
//vfs_mount(NULL, "/proc", DEVPROCFS, &procfs_mount_ops, VFS_MBF_READ_ONLY, 0);
int vfs_mount(struct vnode *cwd, const char *path, device_t dev, struct mount_ops *ops, int mountflags, uid_t uid)
{
    trace("lookup path: %s", path);

    int error;
    struct vnode *vnode = NULL;

    if (uid != 0)
        return -EPERM;
    if (!root_fs) {
        vnode = NULL;
    } else {
#ifdef CONFIG_FAT
        if (dev != DEVFAT2)
#endif
            if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, uid, &vnode)) < 0) {
                trace("not found %s", path);
                return error;
            }
    }
    trace("found path: %s, vnode->ino: %d", path, vnode ? vnode->ino : -1);
    if (vnode && (vnode->bits & VBF_MOUNTED)) {
        vfs_release_vnode(vnode);
        return -EBUSY;
    }

    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        if (mountpoints[i].dev == dev) {
            return -EBUSY;
        }
    }

    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        if (mountpoints[i].dev == 0) {
            mountpoints[i].ops = ops;
            mountpoints[i].mount_node = vnode;
            mountpoints[i].root_node = NULL;
            mountpoints[i].dev = dev;
            mountpoints[i].bits = mountflags;

            if ((error = ops->mount(&mountpoints[i], dev, vnode)) < 0) {
                error("ops->mount is failed");
                mountpoints[i].dev = 0;
                vfs_release_vnode(vnode);
                return error;
            }
#if 0
            if (vnode) {
                trace("mount %s with flags 0x%x to mp[%d] mount_node->ino: %d, fs: %s, dev: 0x%x, bits: 0x%x", path, mountflags, i, mountpoints[i].mount_node->ino, mountpoints[i].ops->fstype, mountpoints[i].dev, mountpoints[i].bits);
            } else {
                trace("mount %s to mp[%d] mount_node is null", path, i);
                trace("set root_fs with mp[%d]", i);
            }
#endif
            if (vnode) {
                vnode->bits |= VBF_MOUNTED;
            } else {
#ifdef CONFIG_FAT
                if (dev != DEVFAT2)
#endif
                root_fs = &mountpoints[i];
            }
            //dump_mp(&mountpoints[i], "vfs_mount");
            return 0;
        }
    }

    return -ENOMEM;
}

int vfs_unmount(device_t dev, uid_t uid)
{
    int error;
    struct mount *mp = NULL;

    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        if (mountpoints[i].dev == dev) {
            mp = &mountpoints[i];
            break;
        }
    }

    if (!mp)
        return -ENODEV;

    if (uid != 0)
        return -EPERM;

    if ((error = mp->ops->sync(mp)) < 0)
        return error;

    if ((error = mp->ops->unmount(mp)) < 0)
        return error;

    if (mp->mount_node) {
        mp->mount_node->bits &= ~VBF_MOUNTED;
        vfs_release_vnode(mp->mount_node);
    } else {
#ifdef CONFIG_FAT
        if (dev != DEVFAT2)
#endif
        root_fs = NULL;
    }
    mp->ops = NULL;
    mp->dev = 0;
    return 0;
}

int vfs_sync(device_t dev)
{
    int error = 0;

    for (int i = 0; i < VFS_MOUNT_MAX; i++) {
        if (mountpoints[i].dev && (!dev || mountpoints[i].dev == dev)) {
            if ((error = mountpoints[i].ops->sync(&mountpoints[i])) < 0)
                return error;
        }
    }
    return 0;
}


int vfs_lookup(struct vnode *cwd, const char *path, int flags, uid_t uid, struct vnode **result)
{
    int error;
    int i = 0, j, k = 0;
    struct mount *mp;
    struct vnode *cur;
    char component[VFS_FILENAME_MAX];

#if 0
    if (cwd) {
        trace("start with [[%s]] cwd->ino: %d, path: %s, flags: 0x%x", cwd->mp->ops->fstype, cwd->ino, path, flags);
    } else {
        trace("start with [[UD]] cwd->ino: -1, path: %s, flags: 0x%x", path, flags);
    }
#endif

    if (!result)
        return -EINVAL;

    cur = (cwd && path[0] != VFS_SEP) ? cwd : root_fs->root_node;
    if (path[0] == VFS_SEP) {
        cur = root_fs->root_node;
        i += 1;
        while (path[i] == VFS_SEP) {
            i += 1;
        }
    }

    /* 変数 i は pathの次に処理する位置 */
    vfs_clone_vnode(cur);
    while (1) {
        k++;
        trace("[[%s]] LP[%d] cur->ino: %d, ref: %d, bits: 0x%x", cur->mp->ops->fstype, k, cur->ino, cur->refcount, cur->bits);
        /* curにファイルシステムがマウントされている場合、curを
         * マウントされているファイルシステムのルートノードに置き換える */
        if (cur->bits & VBF_MOUNTED) {
            mp = _find_mount_by_vnode(cur);
            vfs_release_vnode(cur);
            if (!mp) {
                error("%d has not mount_vnode");
                return -ENXIO;
            }

            cur = vfs_clone_vnode(mp->root_node);
            trace("CHG: cur to [[%s]] %d", cur->mp->ops->fstype, cur->ino);
        }

        // pathの終端にきたら、resultにvnodeをセットして成功で復帰
        if (path[i] == '\0') {
            trace("[[%s]] OK: cur->ino: %d, ref: %d", cur->mp->ops->fstype, cur->ino, cur->refcount);
            *result = cur;
            return 0;
        }

        // curが最後の要素でない場合、curはディレクトリでなければならない
        if (!S_ISDIR(cur->mode)) {
            error("cur (%d) is not dir", cur->ino);
            vfs_release_vnode(cur);
            return -ENOTDIR;
        }

        if ((error = verify_mode_access(uid, R_OK, cur->uid, cur->gid, cur->mode, 0)) < 0) {
            error("uid: %d, gid: %d, mode: 0x%x", cur->uid, cur->gid, cur->mode);
            vfs_release_vnode(cur);
            return -EPERM;
        }

        /* 次の要素名をcomponent[]に読み込む */
        for (j = 0; j < VFS_FILENAME_MAX - 1 && path[i] && path[i] != VFS_SEP; i++, j++)
            component[j] = path[i];
        /* '/'を読み飛ばす */
        while (path[i] == VFS_SEP)
            i += 1;
        /* componentをNULL終端する */
        component[j] = '\0';
        trace("COMP[%d]: '%s'", k, component);
        if (j >= VFS_FILENAME_MAX) {
            vfs_release_vnode(cur);
            trace("componet too long");
            return -ENAMETOOLONG;
        }

        // 最後のコンポーネントの手前で停止する必要がある場合、検索を
        // スキップし、ループの先頭に戻って終了する(curは一つ前のvnode)
        trace("FLG (%s), path[i]: '%c'", flags ? "PARENT" : "SELF", path[i]);
        if ((flags & VLOOKUP_PARENT_OF) && path[i] == '\0') {
            trace("find parent and stop");
            continue;
        }

        // TODO: 現在、マウントされたファイルシステムのルートディレクトリにあり、
        // ".."にアクセスしようとする場合、マウントポイントノードを入れ替える
        trace("fs of cur->ino %d is [[%s]] root_node: %d", cur->ino, cur->mp->ops->fstype,  cur->mp->root_node->ino);
        if (cur == cur->mp->root_node && !strcmp(component, "..") && cur != root_fs->root_node) {
            mp = cur->mp;
            vfs_release_vnode(cur);
            cur = vfs_clone_vnode(mp->mount_node);
            trace("CHG cur to [[%s]]'s mount_node: %d", mp->ops->fstype, mp->mount_node->ino);
        }

        // fs固有のlookupを呼び出して参照されたノードを取得する
        trace("call ops->lookup with [[%s]] cur: 0x%llx (%d), component: %s, &cur: 0x%llx", cur->mp->ops->fstype, cur, cur->ino, component, &cur);
        if ((error = cur->ops->lookup(cur, component, &cur)) < 0) {
            vfs_release_vnode(cur);
            return error;
        }
    }
    trace("not found");
    return -ENOENT;
}

int vfs_reverse_lookup(struct vnode *cwd, char *buf, size_t size, uid_t uid)
{
    int j;
    int len;
    int error;
    struct mount *mp;
    struct vnode *cur;
    struct dirent dir;
    struct vfile *file;
    trace("[[%s]] cwd->ino: %d, buf: 0x%llx, size: 0x%x, uid: %d", (cwd ? cwd->mp->ops->fstype: "ND"), (cwd ? cwd->ino : -1), buf, size, uid);
    if (!cwd) {
        cwd = root_fs->root_node;
        trace("cwd is NUll and set to [[%s]] rootf_fs->rooot_node: ino: %d", cwd->mp->ops->fstype, cwd->ino);
    }

    if (!S_ISDIR(cwd->mode))
        return -ENOTDIR;

    j = size;
    buf[--j] = '\0';
    cur = vfs_clone_vnode(cwd);
    trace("start while loop from [[%s]] cur: 0x%llx (%d), root_fs->root_node: 0x%llx (%d)", cur->mp->ops->fstype, cur, cur->ino, root_fs->root_node, root_fs->root_node->ino);
    while (cur != root_fs->root_node) {
        // カレントノードがファイルシステムのルートノードの場合、
        // そのマウントポイントのvnodeに切り替える。そうしないと
        // inode番号が合わない
        trace("check [[%s]] cur: 0x%llx (%d) with cur->mp->root_node: 0x%llx (%d)", cur->mp->ops->fstype, cur, cur->ino, cur->mp->root_node, cur->mp->root_node->ino);
        if (cur == cur->mp->root_node) {
            trace("change cur from %d to %d", cur->ino, cur->mp->mount_node->ino);
            mp = cur->mp;
            vfs_release_vnode(cur);
            cur = vfs_clone_vnode(mp->mount_node);
            trace("CHG to [[%s]]", cur->mp->ops->fstype);
        }
        trace("cur/..");
        if ((error = vfs_open(cur, "..", O_RDONLY, 0, uid, &file)) < 0) {
            trace("vfs_open error: %d", error);
            vfs_release_vnode(cur);
            return error;
        }

        while (1) {
            if ((error = vfs_readdir(file, &dir)) < 0) {
                vfs_close(file);
                vfs_release_vnode(cur);
                return error;
            } else if (error == 0) {
                break;
            }

            if (dir.ino == cur->ino && strcmp(dir.name, ".") && strcmp(dir.name, "..")) {
                len = strlen(dir.name);
                if (j - len - 1 < 0) {
                    vfs_close(file);
                    vfs_release_vnode(cur);
                    return -1;
                }

                strncpy(&buf[j - len], dir.name, len);
                j -= len;
                buf[--j] = VFS_SEP;
                break;
            }
        }

        vfs_release_vnode(cur);
        cur = vfs_clone_vnode(file->vnode);

        vfs_close(file);
    }
    vfs_release_vnode(cur);

    if (buf[j] != '/')
        buf[--j] = VFS_SEP;

    strncpy(buf, &buf[j], strlen(&buf[j]));
    len = strlen(&buf[j]);
    trace("buf: %s, j: %d, len: %d", buf, j, len);

    return 0;
}


int vfs_access(struct vnode *cwd, const char *path, int mode, uid_t uid, int flags)
{
    int error;
    struct vnode *vnode;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, uid, &vnode)) < 0) {
        trace("lookup error: path: %s, cwd: %d, err: %d", path, cwd->ino, error);
        return error;
    }

    if (mode && verify_mode_access(uid, mode, vnode->uid, vnode->gid, vnode->mode, flags) < 0)
        error = -EPERM;
    else
        error = 0;
    vfs_release_vnode(vnode);
    return error;
}


int vfs_chmod(struct vnode *cwd, const char *path, int mode, uid_t uid)
{
    int error;
    struct vnode *vnode;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;

    if (verify_mode_access(uid, W_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0)
        return -EPERM;

    vnode->mode = (vnode->mode & ~07777) | (mode & 07777);
    error = vnode->ops->update(vnode);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_chown(struct vnode *cwd, const char *path, uid_t owner, gid_t group, uid_t uid)
{
    int error;
    struct vnode *vnode;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;

    if (verify_mode_access(uid, W_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
        vfs_release_vnode(vnode);
        return -EPERM;
    }
    vnode->uid = owner;
    vnode->gid = group;
    error = vnode->ops->update(vnode);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_mknod(struct vnode *cwd, const char *path, mode_t mode, device_t dev, uid_t uid, struct vnode **result)
{
    int error;
    struct vnode *vnode;
    struct vnode *tmp = NULL;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_PARENT_OF, uid, &vnode)) < 0)
        return error;

    if (verify_mode_access(uid, W_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
        vfs_release_vnode(vnode);
        return -EPERM;
    }

    const char *filename = path_last_component(path);
    error = vnode->ops->lookup(vnode, filename, &tmp);
    vfs_release_vnode(tmp);
    if (error == 0) {
        vfs_release_vnode(vnode);
        return -EEXIST;
    } else if (error != -ENOENT) {
        vfs_release_vnode(vnode);
        return error;
    }

    error = vnode->ops->mknod(vnode, filename, mode, dev, uid, result);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_link(struct vnode *oldv, char *oldpath, struct vnode *newv, const char *newpath, uid_t uid)
{
    int error;
    const char *filename;
    struct vnode *vnode = NULL;
    struct vnode *parent = NULL;
    struct vnode *tmpvnode = NULL;

    if ((error = vfs_lookup(oldv, oldpath, VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;

    if ((error = vfs_lookup(newv, newpath, VLOOKUP_PARENT_OF, uid, &parent)) < 0) {
        vfs_release_vnode(vnode);
        return error;
    }

    filename = path_last_component(newpath);

    // targetが存在した場合はerror
    error = parent->ops->lookup(parent, filename, &tmpvnode);
    if (error == -ENOENT) {
        error = 0;
    } else {
        if (!error)
            vfs_release_vnode(tmpvnode);
        error = -EEXIST;
    }

    // 親ディレクトリが書き込み可能かチェック
    if (!error && verify_mode_access(uid, W_OK, parent->uid, parent->gid, parent->mode, 0) < 0)
        error = -EACCES;

    // oldとnewが同じマウントポインにいることをチェック
    if (!error && (vnode->mp != parent->mp))
        error = -EXDEV;

    if (error) {
        vfs_release_vnode(vnode);
        vfs_release_vnode(parent);
        return error;
    }

    error = parent->ops->link(vnode, parent, filename);
    vfs_release_vnode(parent);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_symlink(struct vnode *cwd, const char *oldpath, const char *newpath)
{
    int error;
    struct vnode *parent = NULL;
    const char *filename;

    if ((error = vfs_lookup(cwd, newpath, VLOOKUP_PARENT_OF, thisproc()->uid, &parent)) < 0) {
        error("failed vfs_lookup: cwd: %d, path: %s", cwd->ino, newpath);
        return error;
    }

    // 親ディレクトリが書き込み可能かチェック
    if (verify_mode_access(thisproc()->uid, W_OK, parent->uid, parent->gid, parent->mode, 0) < 0) {
        error("thisproc has not write permission to parent: %d", parent->ino);
        vfs_release_vnode(parent);
        return -EACCES;
    }

    // パーミッションのチェックのためにファイルを検索
    filename = path_last_component(newpath);

    return parent->ops->symlink(parent, oldpath, filename);
}

int vfs_unlink(struct vnode *cwd, const char *path, int flags, uid_t uid)
{
    int error;
    const char *filename;
    struct vnode *parent;
    struct vnode *vnode = NULL;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_PARENT_OF, uid, &parent)) < 0)
        return error;

    // 親ディレクトリが書き込み可能かチェック
    if (verify_mode_access(uid, W_OK, parent->uid, parent->gid, parent->mode, 0) < 0) {
        vfs_release_vnode(parent);
        return -EACCES;
    }

    // パーミッションのチェックのためにファイルを検索
    filename = path_last_component(path);
    if ((error = parent->ops->lookup(parent, filename, &vnode)) < 0) {
        vfs_release_vnode(parent);
        return error;
    }

    // 削除しようとしているファイルが書き込み可能かチェックする
    if (verify_mode_access(uid, W_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
        error = -EPERM;
        goto out;
    }

    // ファイルの削除か、ディレクトリの削除か
    if (flags & AT_REMOVEDIR) {
        if (!S_ISDIR(vnode->mode))
            error = -ENOTDIR;
        else
            error = vnode->ops->rmdir(vnode);
    } else {
        if (S_ISDIR(vnode->mode))
            error = -EISDIR;
        else
            error = parent->ops->unlink(parent, vnode, filename);
    }
out:
    vfs_release_vnode(parent);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_readlink(struct vnode *cwd, const char *path, char *buf, size_t bufsize, uid_t uid)
{
    int error;
    struct vnode *vnode;
    struct vfile *file;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;

    /* pathはsymlinkでない */
    if (S_ISLNK(vnode->mode) == 0) {
        vfs_release_vnode(vnode);
        return -EINVAL;
    }

    //hexdump(buf, 16, "vfs_readlink buffer");

    file = get_vnode(thisproc()->fd_table, vnode);
    if (file) {
        error = file->ops->read(file, buf, bufsize);
    } else if (file = get_file(vnode, O_RDONLY)) {
        error = file->ops->read(file, buf, bufsize);
        //if (error > 0 && error < bufsize)
        //    buf[error] = 0;
        //hexdump(buf, 16, "get_file buffer");
        file->ops->close(file);
    } else {
        error = -ENOENT;
    }
    vfs_release_vnode(vnode);
    return error;
}

static inline int _rename_find_parent(struct vnode *cwd, const char *path, uid_t uid, struct vnode **result)
{
    int error;
    struct vnode *vnode;

    // 親を検索する（ディレクトリでない場合はエラー）
    if ((error = vfs_lookup(cwd, path, VLOOKUP_PARENT_OF, uid, &vnode)) < 0)
        return error;

    // 元ファイルの親ディレクトリが書き込み可で検索可であることをチェック
    if (verify_mode_access(uid, W_OK | X_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
        vfs_release_vnode(vnode);
        return -EACCES;
    }

    *result = vnode;
    return 0;
}

int vfs_rename(struct vnode *oldv, const char *oldpath, struct vnode *newv, const char *newpath, uid_t uid)
{
    int error;
    struct vnode *vnode = NULL;
    const char *oldname, *newname;
    struct vnode *oldparent, *newparent;

    /* oldpathが指定されてない、または、'.', '..'の場合はエラー */
    oldname = path_last_component(oldpath);
    if (*oldname == '\0' || !strcmp(oldname, ".") || !strcmp(oldname, ".."))
        return -EINVAL;

    /* newpathが指定されてない、または、'.', '..'の場合はエラー */
    newname = path_last_component(newpath);
    if (*newname == '\0' || !strcmp(newname, ".") || !strcmp(newname, ".."))
        return -EINVAL;

    /* oldpathの親ディレクトリを検索 */
    if ((error = _rename_find_parent(oldv, oldpath, uid, &oldparent)) < 0)
        return error;

    /* newpathの親ディレクトリを検索 */
    if ((error = _rename_find_parent(newv, newpath, uid, &newparent)) < 0) {
        vfs_release_vnode(oldparent);
        return error;
    }

    /* 両者は同じマウントされたファイルシステムにいなければならない */
    if (oldparent->mp->dev != newparent->mp->dev) {
        vfs_release_vnode(oldparent);
        vfs_release_vnode(newparent);
        return -EXDEV;
    }

    /* oldnameのvnodeを検索 */
    if ((error = oldparent->ops->lookup(oldparent, oldname, &vnode)) < 0) {
        vfs_release_vnode(oldparent);
        vfs_release_vnode(newparent);
        return error;
    }

    /* renameを実行 */
    error = vnode->ops->rename(vnode, oldparent, oldname, newparent, newname);
    vfs_release_vnode(oldparent);
    vfs_release_vnode(newparent);
    vfs_release_vnode(vnode);
    return error;
}

int vfs_open(struct vnode *cwd, const char *path, int flags, mode_t mode, uid_t uid, struct vfile **file)
{
    int error;
    mode_t required_mode = 0;
    struct vnode *vnode = NULL;
    struct vfile *f;

    if (!file)
        return -EINVAL;

    if (!(mode & S_IFMT))
        mode |= S_IFREG;

    trace("[[%s]] cwd: %d, path: %s, create: %d", cwd->mp->ops->fstype, cwd->ino, path, flags & O_CREAT);
    if ((error = vfs_lookup(cwd, path, (flags & O_CREAT) ? VLOOKUP_PARENT_OF : VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;
    trace("path: %s, vnode: %d", path, vnode->ino);
    if (flags & O_CREAT) {
        const char *filename = path_last_component(path);
        if (!path_valid_component(filename)) {
            vfs_release_vnode(vnode);
            return -EINVAL;
        }

        if (verify_mode_access(uid, W_OK, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
                vfs_release_vnode(vnode);
                return -EPERM;
        }
        struct vnode *parent = vnode;

        // パスの最後の要素を検索する。エラー(ENOENT)の場合は新規ファイルを作成する
        if ((vnode->ops->lookup(parent, filename, &vnode)) == -ENOENT) {
            error = parent->ops->create(parent, filename, mode, uid, &vnode);
            trace("created %s with ino: %d under parent: %d", filename, vnode->ino, parent->ino);
            vfs_release_vnode(parent);
            if (error) {
                vfs_release_vnode(vnode);
                return error;
            }
        }
    }

    trace("cwd: ino: %d, rdev: 0x%x, mp->dev: 0x%x, mp->ops: 0x%x; child: %s, ino: %d, rdev: 0x%x, mp->dev: 0x%x, mp->ops: 0x%x", cwd->ino, cwd->rdev, cwd->mp->dev, cwd->mp->ops, path, vnode->ino, vnode->rdev, vnode->mp->dev, vnode->mp->ops);

    // これから使用するvnodeが削除されないようclone
    vfs_clone_vnode(vnode);
    // 処理を進める前にパーミッションをチェックする
    if ((flags & O_ACCMODE) != O_WRONLY)
        required_mode |= R_OK;
    if ((flags & O_ACCMODE) != O_RDONLY)
        required_mode |= W_OK;
    if (verify_mode_access(uid, required_mode, vnode->uid, vnode->gid, vnode->mode, 0) < 0) {
        vfs_release_vnode(vnode);
        return -EPERM;
    }

    if ((f = alloc_file(vnode, flags)) == NULL)
        return -EMFILE;
    if (flags & O_TRUNC && S_ISREG(vnode->mode))
        vnode->ops->truncate(vnode);
    if (flags & O_APPEND && S_ISREG(vnode->mode))
        f->offset = vnode->size;
    if (S_ISCHR(vnode->mode) || S_ISBLK(vnode->mode))
        f->ops = &device_vfile_ops;
    if ((error = f->ops->open(f, flags)) < 0) {
        trace("failed file open");
        free_vfile(f);
    } else {
        if (file)
            *file = f;
    }

    return error;
}

int vfs_close(struct vfile *file)
{
    int error = 0;

    if (file->refcount <= 1)
        error = file->ops->close(file);
    free_vfile(file);
    return error;
}

int vfs_read(struct vfile *file, char *buffer, size_t size)
{
    if ((file->flags & O_ACCMODE) == O_WRONLY)
        return -EACCES;
    return file->ops->read(file, buffer, size);
}

int vfs_write(struct vfile *file, const char *buffer, size_t size)
{
    if ((file->flags & O_ACCMODE) == O_RDONLY)
        return -EACCES;
    return file->ops->write(file, buffer, size);
}

int vfs_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid)
{
    return file->ops->ioctl(file, request, argp, uid);
}

int vfs_poll(struct vfile *file, int events)
{
    return file->ops->poll(file, events);
}

off_t vfs_seek(struct vfile *file, off_t offset, int whence)
{
    return file->ops->seek(file, offset, whence);
}

int vfs_readdir(struct vfile *file, struct dirent *dir)
{
    return file->ops->readdir(file, dir);
}

int vfs_getdents(struct vfile *file, void *buffer, size_t size)
{
    if (!S_ISDIR(file->vnode->mode))
        return -ENOTDIR;

    return file->ops->getdents(file, buffer, size);
}

int vfs_writeback(struct vfile *file, off_t offset, uint64_t addr)
{
    if (file == NULL || !FILE_WRITABLE(file) || !S_ISREG(file->vnode->mode))
        return 0;

    return file->ops->writeback(file, offset, addr);
}

int vfs_fchown(struct vfile *file, uid_t owner, gid_t group)
{
    return file->ops->chown(file, owner, group);
}

int vfs_release_vnode(struct vnode *vnode)
{
    if (!vnode) return 0;
    trace("ino: %d, refcount = %d", vnode->ino, vnode->refcount);
    vnode->refcount--;
    if (vnode->refcount < 0) {
        error("double free of vnode, %x", vnode);
        panic("vfs_release_vnode");
    } else if (vnode->refcount == 0)
        return vnode->ops->release(vnode);
    return 0;
}

const char *path_last_component(const char *path)
{
    int i = strlen(path) - 1;
    if (path[i] == VFS_SEP)
        i--;
    for (; i >= 0; i--) {
        if (path[i] == VFS_SEP)
            break;
    }
    i += 1;
    return &path[i];
}

int path_valid_component(const char *path)
{
    if (*path == '\0')
        return 0;
    for (; *path; path++) {
        if (*path <= 0x20 || *path == VFS_SEP)
            return 0;
    }
    return 1;
}

int verify_mode_access(uid_t current_uid, mode_t require_mode, uid_t file_uid, gid_t file_gid, mode_t file_mode, int flags)
{
    uid_t uid;
    gid_t gid;
    struct proc *p = thisproc();

#if 0
    if (current_uid == 0 || current_uid == file_uid) {
        return require_mode == (require_mode & ((file_mode >> 6) & 0x07));
    } else {
        return require_mode == (require_mode & (file_mode & 0x07));
    }
#endif

   // mode == F_OKの場合はファイルが存在するのでOK
    if (require_mode == 0) return 0;

    // 呼び出し元がrootならR_OK, W_OKは常にOK
    // X_OKはファイルにUGOのいずれかに実行許可があればOK
    if (current_uid == 0) {
        if ((require_mode & X_OK) && !(file_mode & S_IXUGO))
            return -EACCES;
        else
            return 0;
    }

    // root以外は個別に判断
    if (flags & AT_EACCESS) {       // 実効IDで判断
        uid = p->euid;
        gid = p->egid;
    } else {                        // 実IDで判断
        uid = p->uid;
        gid = p->gid;
    }

    if (require_mode & R_OK) {
        if ((file_uid == uid && !(file_mode & S_IRUSR))
         && (file_gid == gid && !(file_mode & S_IRGRP))
                         && !(file_mode & S_IROTH))
        return -EACCES;
    }

    if (require_mode & W_OK) {
        if ((file_uid == uid && !(file_mode & S_IWUSR))
         && (file_gid == gid && !(file_mode & S_IWGRP))
                         && !(file_mode & S_IWOTH))
        return -EACCES;
    }

     if (require_mode & X_OK) {
        if ((file_uid == uid && !(file_mode & S_IXUSR))
         && (file_gid == gid && !(file_mode & S_IXOTH))
                         && !(file_mode & S_IXOTH))
        return -EACCES;
    }

    return 0;
}

static struct vfile *get_file(struct vnode *vnode, int flags)
{
    struct vfile *f;
    int error;
    if ((f = alloc_file(vnode, flags)) == NULL)
        return NULL;
    if ((error = f->ops->open(f, flags)) < 0) {
        error("failed file open");
        free_vfile(f);
        return NULL;
    } else {
        return f;
    }
}


