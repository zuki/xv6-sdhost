# SVR4のvfsシステムの調査

## システムコール関数

- システムコール処理関数は `int syscall(uap, rvp)`の形をしている
- uapは関数ごとに必要な構造体を定義し、そのポインタをuapとして使っている
- rvpは返り値構造体で定義は以下の通り

```c
typedef union rval {
    struct  {
        int r_v1;
        int r_v2;
    } r_v;
    off_t   r_off;
    time_t  r_time;
} rval_t;
```

### システムコール関数の処理例

- システムコール関数 link()はvnode公開関数のvn_link()に処理を投げる
- vn_link()はファイルシステムに依存しない処理をした後、VOP_LINK()でファイルシステムに固有の処理を任せる
- s5link()はs5ファイルシステム固有のlink処理を行う

```c
// fs/vncall.c
struct linka {
	char	*from;
	char	*to;
};

/* ARGSUSED */
int
link(uap, rvp)
	register struct linka *uap;
	rval_t *rvp;
{
	return vn_link(uap->from, uap->to, UIO_USERSPACE);
}

// fs/vnode.c
int
vn_link(from, to, seg)
	char *from;
	char *to;
	enum uio_seg seg;
{
	struct vnode *fvp;		/* from vnode ptr */
	struct vnode *tdvp;		/* to directory vnode ptr */
	struct pathname pn;
	register int error;
	struct vattr vattr;
	long fsid;

	fvp = tdvp = NULL;
	if (error = pn_get(to, seg, &pn))   // fs/pathname.c#pn_get()
		return error;
	if (error = lookupname(from, seg, NOFOLLOW, NULLVPP, &fvp))    // fs/lookup.c#lookupname()
		goto out;

	if (error = lookuppn(&pn, NOFOLLOW, &tdvp, NULLVPP))   // fs/lookup.c#lookuppn()
		goto out;

	/*
	 * Make sure both source vnode and target directory vnode are
	 * in the same vfs and that it is writeable.
	 */
	if (error = VOP_GETATTR(fvp, &vattr, 0, u.u_cred))      // vops#vop_getaatr()
		goto out;
	fsid = vattr.va_fsid;
	if (error = VOP_GETATTR(tdvp, &vattr, 0, u.u_cred))
		goto out;
	if (fsid != vattr.va_fsid) {
		error = EXDEV;
		goto out;
	}
	if (tdvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	/*
	 * Do the link.
	 */
	error = VOP_LINK(tdvp, fvp, pn.pn_path, u.u_cred);      // vnodeops#vop_link
out:
	pn_free(&pn);
	if (fvp)
		VN_RELE(fvp);
	if (tdvp)
		VN_RELE(tdvp);
	return error;
}

// fs/s5/s5vnops.c
struct vnodeops s5vnodeops = {
    [vop_link] = s5link,
};

STATIC int
s5link(tdvp, svp, tname, cr)
	register struct vnode *tdvp;
	struct vnode *svp;
	char *tname;
	struct cred *cr;
{
	register struct inode *tdp, *sip;
	struct vnode *realvp;
	int error;
	char tnm[DIRSIZ+1];

	if (VOP_REALVP(svp, &realvp) == 0)
		svp = realvp;
	sip = VTOI(svp);
	if (svp->v_type == VDIR && !suser(cr))
		return EPERM;
	tdp = VTOI(tdvp);

	/*
	 * Ensure name is truncated to DIRSIZ characters.
	 */
	*tnm = '\0';
	(void) strncat(tnm, tname, DIRSIZ);

	error = direnter(tdp, tnm, DE_LINK, (struct inode *) 0,     // s5dir.c#direnter()
	  sip, (struct vattr *) 0, (struct inode **) 0, cr);
	ITIMES(sip);
	ITIMES(tdp);
	return error;
}
```

## `fs/vncall.c`


## vfs関係の構造体と操作関数

### vfs構造体

```c
typedef struct vfs {
    struct vfs *    vfs_next;           /* next VFS in VFS list */
    struct vfsops * vfs_op;             /* operations on VFS */
    struct vnode *  vfs_vnodecovered;   /* vnode mounted on */
    u_long          vfs_flag;           /* flags */
    u_long          vfs_bsize;          /* native block size */
    int             vfs_fstype;         /* file system type index */
    fsid_t          vfs_fsid;           /* file system id */
    caddr_t         vfs_data;           /* private data */
    dev_t           vfs_dev;            /* device of mounted VFS */
    u_long          vfs_bcount;         /* I/O count (accounting) */
    u_short         vfs_nsubmounts;     /* immediate sub-mount count */
} vfs_t;
```

### ファイルシステムタイプスイッチテーブル構造体

```c
typedef struct vfssw {
    char *          vsw_name;       /* type name string */
    int           (*vsw_init)(struct vfssw *vswp, int fstype);  /* init routine */
    struct vfsops * vsw_vfsops;     /* filesystem operations vector */
    long            vsw_flag;       /* flags */
} vfssw_t;
```

## vfs操作関数

```c
typedef struct vfsops {
    int (*vfs_mount)(struct vfs *vfsp, struct vnode *mvp, struct mounta *uap, struct cred *cr);
    int (*vfs_unmount)(struct vfs *vfsp, struct cred *cr);
    int (*vfs_root)(struct vfs *vfsp, struct vnode **vpp);
    int (*vfs_statvfs)(struct vfs *vfsp, struct statvfs *sp);
    int (*vfs_sync)(struct vfs *vfsp, short flag, struct cred *cr);
    int (*vfs_vget)(struct vfs *vfsp, struct vnode **vpp, struct fid *fidp);
    int (*vfs_mountroot)(struct vfs *vfsp, enum whymountroot why);
    int (*vfs_swapvp)(vfs_t *vfsp, vnode_t **vpp, char *nm);
} vfsops_t;
```

## vnode関係の構造体と操作関数

### vnode構造体

```c
typedef struct vnode {
    u_short             v_flag;             /* vnode flags (see below) */
    u_short             v_count;            /* reference count */
    struct vfs *        v_vfsmountedhere;   /* ptr to vfs mounted here */
    struct vnodeops *   v_op;               /* vnode operations */
    struct vfs *        v_vfsp;             /* ptr to containing VFS */
    struct stdata *     v_stream;           /* associated stream */
    struct page *       v_pages;            /* vnode pages list */
    enum vtype          v_type;             /* vnode type */
    dev_t               v_rdev;             /* device (VCHR, VBLK) */
    caddr_t             v_data;             /* private data for fs */
    struct filock *     v_filocks;          /* ptr to filock list */
} vnode_t;
```

### vnode属性構造体

```c
typedef struct vattr {
    long        va_mask;    /* bit-mask of attributes */
    vtype_t     va_type;    /* vnode type (for create) */
    mode_t      va_mode;    /* file access mode */
    uid_t       va_uid;     /* owner user id */
    gid_t       va_gid;     /* owner group id */
    dev_t       va_fsid;    /* file system id (dev for now) */
    ino_t       va_nodeid;  /* node id */
    nlink_t     va_nlink;   /* number of references to file */
    u_long      va_size0;   /* file size pad (for future use) */
    u_long      va_size;    /* file size in bytes */
    timestruc_t va_atime;   /* time of last access */
    timestruc_t va_mtime;   /* time of last modification */
    timestruc_t va_ctime;   /* time file ``created'' */
    dev_t       va_rdev;    /* device the file represents */
    u_long      va_blksize; /* fundamental block size */
    u_long      va_nblocks; /* # of blocks allocated */
    u_long      va_vcode;   /* version code */
} vattr_t;
```

### vnode操作関数

```c
typedef struct vnodeops {
    int (*vop_open)(struct vnode **vpp, int flag, struct cred *cr));
    int (*vop_close)(struct vnode *vp, int flag, int count, off_t offset, struct cred *cr);
    int (*vop_read)(struct vnode *vp, struct uio *uiop, int ioflag, struct cred *cr);
    int (*vop_write)(struct vnode *vp, struct uio *uiop, int ioflag, struct cred *cr);
    int (*vop_ioctl)(struct vnode *vp, int cmd, int arg, int flag, struct cred *cr, int *rvalp);
    int (*vop_setfl)();
    int (*vop_getattr)(struct vnode *vp, struct vattr *vap, int flags, struct cred *cr);
    int (*vop_setattr)(struct vnode *vp, struct vattr *vap, int flags, struct cred *cr));
    int (*vop_access)(struct vnode *vp, int mode, int flags, struct cred *cr);
    int (*vop_lookup)(struct vnode *dvp, char *name, struct vnode **vpp, struct pathname *pnp, int flags, struct vnode *rdir, struct cred *cr);
    int (*vop_create)(struct vnode *dvp, char *name, struct vattr *vap, enum vcexcl excl, int mode, struct vnode **vpp, struct cred *cr);
    int (*vop_remove)(struct vnode *vp, char *name, struct cred *cr);
    int (*vop_link)(struct vnode *tdvp, struct vnode *svp, char *tname, struct cred *cr);
    int (*vop_rename)(struct vnode *sdvp, char *sname, struct vnode *tdvp, char *tname, char *cr);
    int (*vop_mkdir)(struct vnode *dvp, char *dirname, struct vattr *vap, struct vnode **vpp, struct cred *cr);
    int (*vop_rmdir)(struct vnode *dvp, char *name, struct vnode *cdir, struct cred *cr);
    int (*vop_readdir)(struct vnode *vp, struct uio *uiop, struct cred *cr, int *eofp);
    int (*vop_symlink)(struct vnode *dvp, char *linkname, struct vattr *vap, char *target, struct cred *cr);
    int (*vop_readlink)(struct vnode *vp, struct uio *uiop, struct cred *cr);
    int (*vop_fsync)(struct vnode *vp, struct cred *cr);
    void (*vop_inactive)(struct vnode *vp, struct cred *cr);
    int (*vop_fid)(struct vnode *vp, struct fid **fidpp);
    void (*vop_rwlock)(struct vnode *vp);
    void (*vop_rwunlock)(struct vnode *vp);
    int (*vop_seek)(struct vnode *vp, off_t ooff, off_t *noffp);
    int (*vop_cmp)();
    int (*vop_frlock)(struct vnode *vp, int cmd, struct flock *bfp, int flag, off_t offset, cred_t *cr);
    int (*vop_space)(struct vnode *vp, int cmd, struct flock *bfp, int flag, off_t offset, struct cred *cr);
    int (*vop_realvp)();
    int (*vop_getpage)(struct vnode *vp, u_int off, u_int *protp, struct page *pl[], u_int plsz, struct seg *seg, addr_t addr, enum seg_rw rw, struct cred *cr);
    int (*vop_putpage)(struct vnode *vp, u_int off, int len, flags, struct cred *cr);
    int (*vop_map)(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
    int (*vop_addmap)(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
    int (*vop_delmap)(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
    int (*vop_poll)(vnode_t *vp, short events, int anyyet, short *reventsp, truct pollhead **phpp);
    int (*vop_dump)();
    int (*vop_pathconf)(struct vnode *vp, int cmd, u_long *valp, struct cred *cr);
    int (*vop_allocstore)(struct vnode *vp, u_int off, u_int len, struct cred *cred);
} vnodeops_t;
```
