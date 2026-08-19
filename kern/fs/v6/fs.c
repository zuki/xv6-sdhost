/*
 * File system implementation.  Five layers:
 *   + Blocks: allocator for raw disk blocks.
 *   + Log: crash recovery for multi-step updates.
 *   + Files: inode allocator, reading, writing, metadata.
 *   + Directories: inode with special contents (list of other inodes!)
 *   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
 *
 * This file contains the low-level file system manipulation
 * routines.  The (higher-level) system call implementations
 * are in sysfile.c.
 */

#include <types.h>
#include <mmu.h>
#include <proc.h>
#include <string.h>
#include <console.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <linux/errno.h>

#include <fs/bufcache.h>
#include <vfs.h>
#include <fs/v6/param.h>
#include <fs/v6/fs.h>
#include <fs/v6/file.h>
#include <fs/v6/dir.h>

int mode2v6type(mode_t mode)
{
    int type = mode & S_IFMT;
    switch (type) {
        case S_IFDIR:   return T_DIR;
        case S_IFCHR:   return T_CHR;
        case S_IFBLK:   return T_DEV;
        case S_IFREG:   return T_FILE;
        case S_IFLNK:   return T_SYMLINK;
        case S_IFSOCK:  return T_SOCK;
        case S_IFIFO:   return T_FIFO;
    }
    return T_UNKNOWN;
}

/// v6ファイルシステムの固有データ
//struct v6_vfs v6_vfs;
struct v6_superblock v6_sb = { 0 };

/* v6スーパーブロックをsbに読み込む. */
void v6_readsb(device_t dev, struct v6_superblock *sb)
{
    struct buf *bp;

    bp = get_block(dev, 1, false);
    memmove(&v6_sb, bp->block, sizeof(struct v6_superblock));
    release_block(bp, 0);
}

/* ブロックを0クリア */
void v6_bzero(int dev, int bno)
{
    struct buf *bp;

    bp = get_block(dev, bno, false);
    memset(bp->block, 0, BLKSIZE);
    put_block(bp);
    release_block(bp, 0);
}

/* Blocks. */

/* 0クリアしたブロックを割り当てる. */
uint32_t v6_balloc(uint32_t dev)
{
    int b, bi, m;
    struct buf *bp;

    bp = 0;
    for (b = 0; b < v6_sb.nblocks; b += BPB) {
        trace("BBLOCK(%d): 0x%x", b, BBLOCK(b, v6_sb));
        bp = get_block(dev, BBLOCK(b, v6_sb), false);
        for (bi = 0; bi < BPB && b + bi < v6_sb.size; bi++) {
            m = 1 << (bi % 8);
            if ((bp->block[bi / 8] & m) == 0) {  // Is block free?
                bp->block[bi / 8] |= m;  // Mark block in use.
                put_block(bp);
                release_block(bp, 0);
                v6_bzero(dev, b + bi);
                trace("b: 0x%x, bi: 0x%x, [%d] = 0x%x", b, bi, bp->block[bi / 8]);
                return b + bi;
            }
        }
        release_block(bp, 0);
    }
    panic("balloc: out of blocks");
    return 0;
}

/* ブロックを解放する */
void v6_bfree(device_t dev, uint32_t b)
{
    struct buf *bp;
    int bi, m;

    bp = get_block(dev, BBLOCK(b, v6_sb), false);
    bi = b % BPB;
    m = 1 << (bi % 8);
    if ((bp->block[bi / 8] & m) == 0)
        panic("freeing free block");
    bp->block[bi / 8] &= ~m;
    put_block(bp);
    release_block(bp, 0);
}

/* v6 inode関数
 *
 * inodeは無名の1つのファイルを記述する。
 * inodeもディク上の構造体であるdinodeはファイルのタイプ、サイズ、
 * そのファイルを参照しているリンク数、ファイルコンテンツを保持する
 * ブロックのリストなどのメタデータを保持する。
 *
 * inodeはディスク上のsb.inodestartブロックから順番に並べられる。
 * 各inodeはディスク上の位置を示す番号を持っている。
 *
 * カーネルは複数のプロセスで使用されるinodeへのアクセスを同期する
 * 場所を提供するために使用中のinodeのキャッシュをメモリに保持する。
 * キャッシュされたinodeにはディスクには保存されない管理情報
 * （ip->refとip->valid）が含まれる。
 *
 * inodeとそのインメモリ表現をコードで使用するには一連の状態を
 * 通過する必要がある。
 *
 * * 割り当て: inodeは（dinodeの）タイプが非ゼロの場合に割り当てられる。
 * ialloc()が割り当てを行い、参照カウントとリンクカウントがゼロになった
 * 場合にiput()が解放を行う。
 *
 * * キャッシュの参照: inodeキャッシュのエントリはip->refがゼロに
 * なると解放される。そうでなければ、ip->refはエントリ（オープンファイルと
 * カレントディレクトリ）へのインメモリポインタの数を追跡する。
 * iget()はテーブルエントリを見つけるか作成して、そのrefをインクリメントする。
 * iput()はrefをデクリメントする。
 *
 * * 有効性: inodeキャッシュエントリの情報(タイプ、サイズなど)は
 * ip->validが1のときのみ正しい。ilock()はinodeをディスクから読み込み、
 * ip->validをセットする。一方、iput()はip->refが0になった場合、ip->validを
 * クリアする。
 *
 * * ロック: ファイルシステムのコードはまずinodeをロックしないと、inodeの情報と
 * その内容を調べたり変更したりすることができない。
 *
 * したがって、通常、処理シーケンスは次のようになる。
 *   ip = iget(dev, ino)
 *   ilock(ip)
 *   ... examine and modify ip->xxx ...
 *   iunlock(ip)
 *   iput(ip)
 *
 * ilock()とiget()が分離されているのでシステムコールは(オープンファイルなどの）
 * inodeへの長期的な参照を取得することができ、(read()などで)短期間だけロック
 * する。この分離はパス名検索の際のデッドロックや競合を避けるのにも役立っている
。
 * iget()はip->refをインクリメントするのでinodeはキャッシュに留まり、それへの
 * ポインタの有効性が保持される。
 *
 * 内部のファイルシステム関数の多くは呼び出し元が関係するinodeをロックして
 * いることを想定している。これにより呼び出し元は多段階のアトミック操作が
 * 可能である。
 *
 * スピンロックicache.lockはicacheエントリの割り当てを保護する。ip->refは
 * エントリがフリーかどうかを示し、ip->rdevとip->inoはエントリが保持する
 * inodeを示すので、これらのフィールドのいずれかを使用している間、
 * icache.lockを保持しなければならない。
 *
 * スリープロックip->lockはref、dev、ino以外のすべてのipフィールドを保護する。
 * inodeのip->valid、ip->size、ip->typeなどを読み書きするにはip->lockを保持
 * しなければならない。
 */

/* インメモリinodeキャッシュ */
struct _v6_icache {
    struct spinlock lock;
    struct v6_inode inode[NINODE];
} v6_icache;

/* v6 inodeシステムを初期化する */
void v6_iinit(device_t dev)
{
    int i = 0;

    /* 1. インメモリinodeキャッシュの初期化*/
    initlock(&v6_icache.lock, "inode");
    for (i = 0; i < NINODE; i++) {
        initsleeplock(&v6_icache.inode[i].lock, "inode");
    }

#if 0
    v6_readsb(dev, &v6_sb);
    info("v6_sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
 inodestart %d bmapstart %d", v6_sb.size, v6_sb.nblocks, v6_sb.ninodes, v6_sb.nlog, v6_sb.logstart, v6_sb.inodestart, v6_sb.bmapstart);
#endif
}

void v6_set_super(void)
{
    v6_readsb(DEVV6, &v6_sb);
    info("v6_sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
 inodestart %d bmapstart %d", v6_sb.size, v6_sb.nblocks, v6_sb.ninodes, v6_sb.nlog, v6_sb.logstart, v6_sb.inodestart, v6_sb.bmapstart);
    get_rootfs()->super = &v6_sb;
}

extern struct vnode_ops v6_vnode_ops;

/* onディスクdinodeを割り当てる.
 *
 * typeフィールでの値を設定することで割り当て済みとマークする。
 */
struct v6_inode *v6_ialloc(struct mount *mp, uint16_t type)
{
    int ino;
    struct buf *bp;
    struct v6_dinode *dip;

    for (ino = 1; ino < v6_sb.ninodes; ino++) {
        bp = get_block(mp->dev, IBLOCK(ino, v6_sb), false);
        //bp = get_block(mp->dev, IBLOCK(ino), false);
        dip = (struct v6_dinode *)bp->block + ino % IPB;
        if (dip->type == 0) {   // a free inode
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            put_block(bp);      // mark it allocated on the disk
            release_block(bp, 0);
            return v6_iget(mp, ino);
        }
        release_block(bp, 0);
    }
    panic("ialloc: no inodes");
    return 0;
}

/* 変更したインメモリinodeをディスクに書き出す.
 *
 * ディスクにあるすべてのip-xxxフィールドを変更した後に
 * 呼び出す必要がある。v6_inodeはwrite-throughだからである.
 * 呼び出し元は ip->lock を保持する必要がある
 */
void v6_iupdate(struct v6_inode *ip)
{
    struct buf *bp;
    struct v6_dinode *dip;
    struct vnode *vp = ITOV(ip);

    trace("type: %d, mode: 0x%x, valid: %d", ip->type, vp->mode, ip->valid);
    if ((!S_ISBLK(vp->mode) && !S_ISCHR(vp->mode)) || ip->valid == 0) {
        bp = get_block(vp->rdev, IBLOCK(vp->ino, v6_sb), false);
        dip = (struct v6_dinode *)(bp->block + vp->ino % IPB);
        dip->nlink = vp->nlink;
        dip->type  = ip->type;
        dip->rdev  = vp->rdev;
        dip->size  = vp->size;
        dip->mode  = vp->mode;
        dip->uid   = vp->uid;
        dip->gid   = vp->gid;

        memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
        mark_block_dirty(bp);
        put_block(bp);
        release_block(bp, 0);
        dmb();
    }
}

/*
 * デバイスdev上のinode番号がinoのv6_dinodeを見つけて、
 * そのインメモリコピーを返す。
 * v6_inodeはロックしない、また、ディスクから読み込まない.
 */
struct v6_inode *v6_iget(struct mount *mp, uint32_t ino)
{
    struct v6_inode *ip, *empty;
    struct vnode *vp;
    trace("dev: 0x%x, ino: %d", mp->dev, ino);

    acquire(&v6_icache.lock);
    debug("acquire v6_icache.lock for ino %d", ino);
    // v6_inodeがキャッシュされているかチェックする
    empty = NULL;
    for (ip = &v6_icache.inode[0]; ip < &v6_icache.inode[NINODE]; ip++) {
        vp = ITOV(ip);
        if (vp->refcount > 0 && vp->rdev == mp->dev && vp->ino == ino) {
            vp->refcount++;
            release(&v6_icache.lock);
            debug("hit: release v6_icache.lock for ino %d", ino);
            trace("hit ip->ino: %d", vp->ino);
            return ip;
        }
        if (empty == NULL && vp->refcount == 0)
            empty = ip;
    }

    if (empty == NULL) {
        trace("no v6_inodes");
        release(&v6_icache.lock);
        debug("no inodes: release v6_icache.lock for ino %d", ino);
        return NULL;
    }
    // v6_inodeキャッシュエントリをリサイクル.
    ip = empty;
    trace("mp: 0x%x, mount_node: 0x%x, dev: 0x%x", mp, mp->mount_node, mp->dev);
    vfs_init_vnode(&ip->vnode, &v6_vnode_ops, mp, 0, 1, 0, 0, mp->dev, ino, 0, FSV6, 0, 0, 0);
    ip->valid = 0;
    ITOV(ip)->data = ip;
    //v6_dump(ip, "v6_iget");
    release(&v6_icache.lock);
    debug("release v6_icache.lock for ino %d", ino);
    return ip;
}

/*
 * 参照カウンタを増分する.
 * ip = idup(ip1) イディオムが使えるようにipを返す.
 */
struct v6_inode *v6_idup(struct v6_inode *ip)
{
    acquire(&v6_icache.lock);
    ITOV(ip)->refcount++;
    release(&v6_icache.lock);
    return ip;
}

// TODO: v6_inode, v6_dinodeの各種timeの適切な更新
/*
 * 指定されたv6_inodeをロックする.
 * 必要に応じてディスクからinodeを読み込む.
 */
void v6_ilock(struct v6_inode *ip)
{
    struct buf *bp;
    struct v6_dinode *dip;
    struct vnode *vp;

    if (ip == 0 || ITOV(ip)->refcount < 0)
        panic("v6_ilock");

    acquiresleep(&ip->lock);
    debug("acquiresleep for ino=%d", ITOV(ip)->ino);
    vp = ITOV(ip);
    trace("vp->ino: %d, rdev: 0x%x, valid: %d, type: %d, mode: 0x%x", vp->ino, vp->rdev, ip->valid, ip->type, vp->mode);

    if (ip->valid == 0) {
        trace("bno: 0x%x, v6_sb: 0x%x, inostart: 0x%x", IBLOCK(vp->ino, v6_sb), v6_sb, v6_sb.inodestart);
        bp = get_block(vp->rdev, IBLOCK(vp->ino, v6_sb), false);
        trace("got bp");
        dip = (struct v6_dinode *)bp->block + (vp->ino % IPB);
        ip->type  = dip->type;
        vp->nlink = dip->nlink;
        vp->rdev  = dip->rdev;
        vp->size  = dip->size;
        vp->mode  = dip->mode;
        vp->uid   = dip->uid;
        vp->gid   = dip->gid;
        memmove(&vp->atime, &dip->atime, sizeof(struct timespec));
        memmove(&vp->mtime, &dip->mtime, sizeof(struct timespec));
        memmove(&vp->ctime, &dip->ctime, sizeof(struct timespec));
        memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
        trace("type: %d, nlink: %d, rdev: 0x%x, size: 0x%x, mode: 0x%x, addrs[0]: 0x%x", ip->type, vp->nlink, vp->rdev, vp->size, vp->mode, ip->addrs[0]);
        put_block(bp);
        release_block(bp, 0);
        ip->valid = 1;
        if (ip->type == 0)
            panic("ilock: no type");
    }
    trace("ok");
}

/* 指定されたv6_inodeのロックを外す. */
void v6_iunlock(struct v6_inode *ip)
{
    if (ip == 0 || !holdingsleep(&ip->lock) || ITOV(ip)->refcount < 0)
        panic("iunlock");

    releasesleep(&ip->lock);
    debug("releasesleep: ino=%d", ITOV(ip)->ino);
}

/* インメモリv6_inodeの参照カウンタを減ずる.
 *
 * それが最後の参照だった場合、そのキャッシュエントリはリサイクルが
 * 可能になる。そしてそのv6_inodeがlinkを持たない場合、そのv6_dinodeと
 * その内容をディスクから解放する.
 */
void v6_iput(struct v6_inode *ip)
{
    struct vnode *vp = ITOV(ip);

    acquiresleep(&ip->lock);
    debug("acquiresleep: ino=%d", ITOV(ip)->ino);
    if (ip->valid && vp->nlink == 0) {
        //acquire(&v6_icache.lock);
        int r = vp->refcount;
        //release(&v6_icache.lock);
        if (r == 1) {
            /* v6_inodeはリンクを持たず、参照もないのでデータを
             * 切り詰めて解放する */
            v6_itrunc(ip);
            ip->type = 0;
            v6_iupdate(ip);
            ip->valid = 0;
        }
    }

    releasesleep(&ip->lock);
    debug("releasesleep: ino=%d", ITOV(ip)->ino);

    // refcountはローカルではさわらない vfs_release_vnode()で行う
}

/* よくあるイディオム: unlockしてputする */
void v6_iunlockput(struct v6_inode *ip)
{
    v6_iunlock(ip);
    v6_iput(ip);
}

/* inodeのコンテンツ
 *
 * 各inodeに関するコンテンツ（データ）はディスクのブロックに格納
 * されている。最初のNDIRECT個のブロック数はip->addrs[]に記載されて
 * いる。次のNINDIRECT個のブロックはブロックip->ddrs[NDIRECT]に
 * 記載されている。
 *
 * inode ipのn番目のブロックのブロックアドレスを返す。そのような
 * ブロックがない場合は割り当てて返す。
 */
uint32_t v6_bmap(struct v6_inode *ip, uint32_t bn)
{
    uint32_t idx1, idx2, addr, *a;
    struct buf *bp;
    struct vnode *vp = ITOV(ip);

    if (bn < NDIRECT) {
        if ((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = v6_balloc(vp->rdev);
        if (vp->ino == 17)
        trace("[%d] addr: %x", thisproc()->pid, addr);
        return addr;
    }
    bn -= NDIRECT;

    if (bn < NINDIRECT) {
        // Load indirect block, allocating if necessary.
        if ((addr = ip->addrs[NDIRECT]) == 0) {
            addr = v6_balloc(vp->rdev);
            if (addr == 0)
                return 0;
            ip->addrs[NDIRECT] = addr;
        }
        bp = get_block(vp->rdev, addr, false);
        a = (uint32_t*)bp->block;
        if ((addr = a[bn]) == 0) {
            addr = v6_balloc(vp->rdev);
            if (addr == 0)
                return 0;
            a[bn] = addr;
            put_block(bp);
        }
        trace("bn: %d, addr: 0x%llx", bn, addr);
        release_block(bp, 0);
        return addr;
    }

    bn -= NINDIRECT;

    if (bn < NINDIRECT2) {
        // Load indirect block, allocating if necessary.
        if ((addr = ip->addrs[NDIRECT+1]) == 0) {
            addr = v6_balloc(vp->rdev);
            if (addr == 0)
                return 0;
            ip->addrs[NDIRECT+1] = addr;
        }
        idx1 = bn / NINDIRECT;
        idx2 = bn % NINDIRECT;
        bp = get_block(vp->rdev, addr, false);
        a = (uint32_t*)bp->block;
        if ((addr = a[idx1]) == 0) {
            addr = v6_balloc(vp->rdev);
            if (addr == 0)
                return 0;
            a[idx1] = addr;
            put_block(bp);
        }
        trace("idx1: %d, addr1: 0x%llx", idx1, addr);
        release_block(bp, 0);
        bp = get_block(vp->rdev, addr, false);
        a = (uint32_t*)bp->block;
        if ((addr = a[idx2]) == 0) {
            addr = v6_balloc(vp->rdev);
            if (addr == 0)
                return 0;
            a[idx2] = addr;
            put_block(bp);
        }
        trace("idx2: %d, addr2: 0x%llx", idx2, addr);
        release_block(bp, 0);
        return addr;
    }
    panic("bmap: out of range");
    return 0;
}

/* inodeを切り詰める（コンテンツを破棄する）.
 *
 * そのinodeにリンクするinodeがなく（参照するディレクトリ
 * エントリがない）そのインメモリinodeへの参照がない
 * （オープンされたファイルやカレントディレクトリでない）
 * 場合にのみ呼び出される。
 */
void v6_itrunc(struct v6_inode *ip)
{
    int i, j;
    uint32_t *a, *b;
    struct buf *bp;
    struct vnode *vp = ITOV(ip);

    for (i = 0; i < NDIRECT; i++) {
        if (ip->addrs[i]) {
            v6_bfree(vp->rdev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    if (ip->addrs[NDIRECT]) {
        bp = get_block(vp->rdev, ip->addrs[NDIRECT], false);
        a = (uint32_t *) bp->block;
        for (int j = 0; j < NINDIRECT; j++) {
            if (a[j])
                v6_bfree(vp->rdev, a[j]);
        }
        release_block(bp, 0);
        v6_bfree(vp->rdev, ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }

    if (ip->addrs[NDIRECT+1]) {
        bp = get_block(vp->rdev, ip->addrs[NDIRECT+1], false);
        a = (uint32_t *)bp->block;
        for (i = 0; i < NINDIRECT; i++) {
            if (a[i]) {
                release_block(bp, 0);
                bp = get_block(vp->rdev, a[i], false);
                b = (uint32_t *)bp->block;
                for (j = 0; j < NINDIRECT; j++) {
                    if (b[j]) {
                        v6_bfree(vp->rdev, b[j]);
                        b[j] = 0;
                    }
                }
                v6_bfree(vp->rdev, a[i]);
                a[i] = 0;
            }
        }
        release_block(bp, 0);
        v6_bfree(vp->rdev, ip->addrs[NDIRECT+1]);
        ip->addrs[NDIRECT+1] = 0;
    }

    vp->size = 0;
    v6_iupdate(ip);
}


/*
 * v6_inodeからデータを読み込むRead data from inode.
 * Caller must hold ip->lock.
 */
size_t v6_readi(struct v6_inode *ip, char *dst, off_t off, size_t n)
{
    size_t tot, m;
    struct buf *bp;
    struct vnode *vp = ITOV(ip);

    if (off > vp->size || off + n < off)
        return -EFAULT;
    if (off + n > vp->size)
        n = vp->size - off;
    trace("ip: ino:%x, rdev: 0x%x, off: 0x%x, n: %d", vp->ino, vp->rdev, off, n);
    for (tot = 0; tot < n; tot += m, off += m, dst += m) {
        bp = get_block(vp->rdev, v6_bmap(ip, off / BLKSIZE), false);
        m = MIN(n - tot, BLKSIZE - off % BLKSIZE);
        memmove(dst, bp->block + off % BLKSIZE, m);
        release_block(bp, 0);
    }

    return n;
}

/*
 * v6_inodeにデータを書き込む.
 * 呼び出し元はip->lock を保持する必要がある。
 */
size_t v6_writei(struct v6_inode *ip, const char *src, off_t off, size_t n)
{
    size_t tot, m;
    struct buf *bp;
    struct vnode *vp = ITOV(ip);
    trace("[%d] ip: ino: %d, rdev: 0x%x, size: 0x%x: off: 0x%x, n: %d", thisproc()->pid, vp->ino, vp->rdev, vp->size, off, n);

    if (off > vp->size || off + n < off)
        return -EFAULT;
    if (off + n > MAXFILE * BLKSIZE)
        return -EFBIG;

    for (tot = 0; tot < n; tot += m, off += m, src += m) {
        bp = get_block(vp->rdev, v6_bmap(ip, off / BLKSIZE), false);
        m = MIN(n - tot, BLKSIZE - off % BLKSIZE);
        memmove(bp->block + off % BLKSIZE, src, m);
        trace("put_block: dev: 0x%x, bno: 0x%x", bp->dev, bp->blockno);
        put_block(bp);
        release_block(bp, 0);
    }
    dmb();

    if (n > 0 && off > vp->size) {
        vp->size = off;
        trace("iupdate with ino: %d, rdev: 0x%x", ITOV(ip)->ino, ITOV(ip)->rdev);
        v6_iupdate(ip);
    }
    return n;
}

/* ディレクトリ関連の関数. */

/*
 * 指定のディレクトリでディレクトリエントリを探す.
 * 見つかったらそのv6_inodeを返す。見つからなかった
 * 場合はNULLを返す。
 */
struct v6_inode *v6_dirlookup(struct v6_inode *dp, const char *name)
{
    struct dirent de;
    struct vnode *vp = ITOV(dp);

    trace("lookup %s in ino: %d", name, vp->ino);

    if (!S_ISDIR(vp->mode)) {
        trace("dirlookup not DIR: dp->ino: 0x%x, mode: 0x%x, name: %s", vp->ino, vp->mode, name);
        return NULL;
    }

    if ((dir_find_entry_by_name(vp, name, &de, 0)) < 0) {
        trace("no entry found by name: %s", name);
        return NULL;
    }
    trace("found by name: %s, dev: 0x%x, ino: %d", name, vp->mp->dev, de.ino);
    return v6_iget(vp->mp, de.ino);
}

/* 新規ディレクトリエントリ (name, ino, type) をディレクトリdpに書き込む. */
int v6_dirlink(struct v6_inode *dp, const char *name, uint32_t ino, uint16_t type)
{
    off_t off;
    struct dirent de;
    struct v6_inode *ip;
    struct vnode *vp = ITOV(dp);

    trace("dp->ino: %d, name: %s", vp->ino, name);
    /* nameがすでに存在しないかチェックし、あればエラー. */
    if ((ip = v6_dirlookup(dp, name)) != NULL) {
        v6_iput(ip);
        return -EEXIST;
    }

    /* 空きディレクトリエントリがあればそれを使用し、なければ
     * vpを拡張（次のブロックを使用）して追加する */
    for (off = 0; off < vp->size; off += DESIZE) {
        if (v6_readi(dp, (char *)&de, off, DESIZE) != DESIZE) {
            return -EIO;
        }
        if (de.ino == 0) {
            break;
        }
    }

    strncpy(de.name, name, DIRSIZ);
    de.ino = ino;
    de.type = type;
    trace("write to dp->ino: %d, rdev: 0x%x with de: ino=%d, type=%d, name=%s, off=0x%x", vp->ino, vp->rdev, de.ino, de.type, de.name, off);
    if (v6_writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
       return -EIO;

    return 0;
}

void sync_v6_inodes(void)
{
    acquire(&v6_icache.lock);
    for (int i = 0; i < NINODE; i++) {
        if (v6_icache.inode[i].valid == 1) {
            release(&v6_icache.lock);
            acquiresleep(&v6_icache.inode[i].lock);
            v6_iupdate(&v6_icache.inode[i]);
            releasesleep(&v6_icache.inode[i].lock);
            acquire(&v6_icache.lock);
        }
    }
    release(&v6_icache.lock);
}

void v6_dump(struct v6_inode *inode, const char *title)
{
    struct vnode *vnode = ITOV(inode);
    cprintf("=== dump %s: 0x%x ===\n", title, vnode);
    cprintf("   ops: 0x%x\n", vnode->ops);
    cprintf("    mp: 0x%x\n", vnode->mp);
    cprintf("   ref: 0x%x\n", vnode->refcount);
    cprintf("  mode: 0x%x\n", vnode->mode);
    cprintf(" nlink: 0x%x\n", vnode->nlink);
    cprintf("  bits: 0x%x\n", vnode->bits);
    cprintf("  rdev: 0x%x\n", vnode->rdev);
    cprintf("   ino: 0x%x\n", vnode->ino);
    cprintf("  size: 0x%x\n", vnode->size);
    cprintf("  data: 0x%x\n", vnode->data);
    cprintf("inode : 0x%x\n", inode);
    cprintf(" vnode: 0x%x\n", ITOV(inode));
    cprintf(" valid: %d\n", inode->valid);
    cprintf("  type: %d\n", inode->type);
    cprintf("==========================\n", vnode);
}
