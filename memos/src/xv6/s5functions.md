# SVR5の調査

## s5ファイルシステムの関数

### s5alloc.c

```c
int blkalloc(struct vfs *vfsp, daddr_t *bnp);
void blkfree(struct vfs *vfsp, daddr_t bno);
int badblock(struct filsys *fp, daddr_t bn, dev_t dev);
int ialloc(struct vfs *vfsp, u_short mode, int nlink, dev_t rdev, int uid, int gid, struct inode **ipp);
void ifree(struct inode *ip);
int s5freesp(struct vnode *vp, struct flock * lp, int flag);
void s5indirtrunc(struct vfs *vfsp, daddr_t bn, daddr_t lastbn, int level);
```

### s5blocklist.c

```c
int s5allocmap(struct inode *ip)
STATIC int s5bldblklst(int *lp, struct inode *ip, int nblks);
STATIC int s5bldindr(struct inode *ip, int **lp, int *eptr, dev_t dev, int blknbr, int indlvl);
int s5freemap(struct inode *ip);
```

### s5bmap.c

```c
int bmap(inode_t *ip, daddr_t lbn, daddr_t *bnp, daddr_t *rabnp, enum seg_rw rw, int alloc_only);
int bmapalloc(struct inode *ip, daddr_t first, daddr_t last, int alloc_only, daddr_t *dblist);
```

### s5dir.c

```c
int dirlook(struct inode *dp, char *namep, struct inode **ipp, struct cred *cr);
int direnter(struct inode * tdp, char *namep, enum de_op op, truct inode *sdp, sip, struct vattr *vap, struct inode **ipp, struct cred *cr);
int dirremove(struct inode *dp, namep, struct inode *oip, struct vnode *cdir, enum dr_op op, struct cred *cr);

int dircheckforname(struct inode *tdp, char *namep, off_t *offp, struct inode **tipp);
int dirsearch(struct inode *dip, char *comp, struct inode **ipp, off_t *offp);
int dirrename(struct inode *sdp, struct inode *sip, struct inode *tdp, char *namep, struct inode *tip, off_t offset, struct cred *cr)
int dirfixdotdot(struct inode *dp, struct inode *opdp, struct inode *npdp);
int diraddentry(truct inode *tdp, char *namep, off_t offset, truct inode *sip, truct inode *sdp, enum de_op op);
int dirmakeinode(struct inode *tdp, struct inode **ipp, struct vattr *vap, enum de_op op, struct cred *cr);
int dirmakedirect(struct inode *ip, struct inode *dp)
int dirempty(struct inode *ip, int *dotflagp);
int dircheckpath(struct inode *source, struct inode *target);
```

### s5inode.c

```c
void inoinit(void);
int iget(struct vfs *vfsp, int ino, struct inode **ipp);
void iput(struct inode *ip);
void iinactive(struct inode *ip, struct cred *cr);
int iflush(struct vfs *vfsp, int force);
STATIC void ipfree(struct inode *ip);
void iunhash(struct inode *ip);
int iread(struct inode *ip, int ino);
void iupdat(struct inode *ip);
void iuptimes(struct inode *ip);
int itrunc(struct inode *ip);
void tloop(truct vfs *vfsp, daddr_t bn, int f1, int f2)
void ilock(struct inode *ip);
void iunlock(struct inode *ip);
void s5init(struct vfssw *vswp, int fstype);
int iaccess(struct inode *ip, int mode, struct cred *cr);
int syncip(struct inode *ip, int flags);
void inull(struct vfs *vfsp);
```

### s5rdwri.c

```c
int rdwri(enum uio_rw rw, struct inode *ip, caddr_t base, int len, offset, enum uio_seg seg, int ioflag, int aresid);
int readi(struct inode *ip, struct uio *uiop, int ioflag);
int writei(struct inode *ip, struct uio *uiop, int ioflag);
```

### s5vfsops.c

```c
struct vfsops s5vfsops = {
 s5mount,
 s5unmount,
 s5root,
 s5statvfs,
 s5sync,
 s5vget,
 s5mountroot,
 fs_nosys, /* swapvp */
 fs_nosys, /* filler */
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
};

STATIC int s5mount(struct vfs *vfsp, struct vnode *mvp, struct mounta *uap, struct cred *cr);
STATIC int s5unmount(struct vfs *vfsp, struct cred *cr);
STATIC int s5root(struct vfs *vfsp, struct vnode **vpp);
STATIC int s5statvfs(struct vfs *vfsp, struct statvfs *sp);
STATIC int s5sync(struct vfs *vfsp, short flag, struct cred *cr);
STATIC int s5vget(struct vfs *vfsp, struct vnode **vpp, struct fid *fidp);
STATIC int s5mountroot(struct vfs *vfsp, enum whymountroot why);

STATIC void s5update(void);
STATIC int s5flushi(short flag);
STATIC void s5flushsb(struct vfs *vfsp);
STATIC int s5vfs_init(struct s5vfs *s5vfsp, int bsize);
```

### s5vnops.c

```c
struct vnodeops s5vnodeops = {
 s5open,
 s5close,
 s5read,
 s5write,
 s5ioctl,
 fs_setfl,
 s5getattr,
 s5setattr,
 s5access,
 s5lookup,
 s5create,
 s5remove,
 s5link,
 s5rename,
 s5mkdir,
 s5rmdir,
 s5readdir,
 s5symlink,
 s5readlink,
 s5fsync,
 s5inactive,
 s5fid,
 s5rwlock,
 s5rwunlock,
 s5seek,
 fs_cmp,
 s5frlock,
 s5space,
 fs_nosys, /* realvp */
 s5getpage,
 s5putpage,
 s5map,
 s5addmap,
 s5delmap,
 fs_poll,
 fs_nosys, /* dump */
 fs_pathconf,
 s5allocstore,
 fs_nosys, /* filler */
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
 fs_nosys,
};

STATIC int s5open(struct vnode **vpp, int flag, struct cred *cr);
STATIC int s5close(struct vnode *vp, int flag, int count, off_t offset, struct cred *cr);
STATIC int s5read(struct vnode *vp, struct uio *uiop, int ioflag, struct cred *cr);
STATIC int s5write(struct vnode *vp, struct uio *uiop, int ioflag, struct cred *cr);
STATIC int s5ioctl(struct vnode *vp, int cmd, int arg, int flag, struct cred *cr, int *rvalp);
STATIC int s5getattr(struct vnode *vp, struct vattr *vap, int flags, struct cred *cr);
STATIC int s5setattr(struct vnode *vp, struct vattr *vap, int flags, struct cred *cr);
STATIC int s5access(struct vnode *vp, int mode, int flags, struct cred *cr);
STATIC int s5lookup(struct vnode *dvp, char *name, struct vnode **vpp, struct pathname *pnp, int flags, struct vnode *rdir, struct cred *cr);
STATIC int s5create(struct vnode *dvp, char *name, struct vattr *vap, enum vcexcl excl, int mode, struct vnode **vpp, struct cred *cr);
STATIC int s5remove(struct vnode *vp, char *name, struct cred *cr);
STATIC int s5link(struct vnode *tdvp, struct vnode *svp, char *tname, struct cred *cr);
STATIC int s5rename(struct vnode *sdvp, char *sname, struct vnode *tdvp, char *tname, char *cr);
STATIC int s5mkdir(struct vnode *dvp, char *dirname, struct vattr *vap, struct vnode **vpp, struct cred *cr);
STATIC int s5rmdir(struct vnode *dvp, char *name, struct vnode *cdir, struct cred *cr);
STATIC int s5readdir(struct vnode *vp, struct uio *uiop, struct cred *cr, int *eofp);
STATIC int s5symlink(struct vnode *dvp, char *linkname, struct vattr *vap, char *target, struct cred *cr);
STATIC int s5readlink(struct vnode *vp, struct uio *uiop, struct cred *cr);
STATIC int s5fsync(struct vnode *vp, struct cred *cr);
STATIC void s5inactive(struct vnode *vp, struct cred *cr);
STATIC int s5fid(struct vnode *vp, struct fid **fidpp);
STATIC void s5rwlock(struct vnode *vp);
STATIC void s5rwunlock(struct vnode *vp);
STATIC int s5seek(struct vnode *vp, off_t ooff, off_t *noffp);
STATIC int s5frlock(struct vnode *vp, int cmd, struct flock *bfp, int flag, off_t offset, cred_t *cr);
STATIC int s5space(struct vnode *vp, int cmd, struct flock *bfp, int flag, off_t offset, struct cred *cr);
STATIC int s5getsp(struct vnode *vp, u_long *totp);
STATIC int s5getapage(struct vnode *vp, u_int off, u_int *protp, struct page *pl[], u_int plsz, struct seg *seg, addr_t addr, enum seg_rw rw, struct cred *cr);
STATIC int s5getpage(struct vnode *vp, u_int off, u_int *protp, struct page *pl[], u_int plsz, struct seg *seg, addr_t addr, enum seg_rw rw, struct cred *cr);
STATIC int s5putpage(struct vnode *vp, u_int off, int len, flags, struct cred *cr);
STATIC int s5map(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
STATIC int s5addmap(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
STATIC int s5delmap(struct vnode *vp, u_intoff, struct as *as, caddr_t addrp, u_int len, u_int prot, u_int maxprot, u_int flags, struct cred *cr);
STATIC int s5allocstore(struct vnode *vp, u_int off, u_int len, struct cred *cred);

STATIC int s5writelbn(struct inode *ip, daddr_t bn, struct page *pp, u_int len, u_int pgoff, int flags);
STATIC void filldir(char *direntp, int nmax, char *directp, int nleft, off_t diroff, int *ndirentp, int *ndirectp);
```

## fsディレクトリ

### fs_subr.c

```c
int fs_frlock(vnode_t *vp, int cmd, struct flock *bfp, int flag, off_t offset, cred_t *cr);
int fs_poll(vnode_t *vp, short events, int anyyet, short *reventsp, truct pollhead **phpp);
int fs_vcode(vnode_t *vp, u_long *vcp);
int fs_pathconf(struct vnode *vp, int cmd, u_long *valp, struct cred *cr);

int fs_nosys(void) { return ENOSYS; }
int fs_sync(struct vfs *vfsp, short flag, cred_t *cr) { return 0; }
void fs_rwlock(vnode_t *vp) {}
void fs_rwunlock(vnode_t *vp) {}
int fs_cmp(vnode_t *vp1, vnode_t *vp2) { return vp1 == vp2; }
int fs_setfl(vnode_t *vp, int oflags, int nflags, cred_t *cr) { return 0; }
```
