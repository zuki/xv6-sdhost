## s5dir.h

```c
	struct direct {};
	#define DIRSIZ
	#define SDSIZ
```

## s5fblk.h

```C
	struct fblk { int df_nfree; daddr_t df_free[NCFREE]; }
```

## s5filsys.h

```c
	struct filsys {};	// superblock
	#define FsXXX;
	#define getfs(vfsp) ((struct filsys *)((struct s5vfs *)vfsp->vfs_data)->vfs_bufp->b_un.b_addr)
```

## s5ino.h

```c
	struct dinode {};		// directory inode
```

## s5inode.h

```c
	struct inode {};		// on-memory inode
	struct ifreelist {};	// free inode list
	#define IXXX;			// inode flags : IACC/ICHG/ISYN など
	#define IFXXX;			// File types : stat.hのS_IFXXXと同じ
	#define ISXXX;			// File modes : = VSXXX かつ stat.hのS_ISXXXと同じ
	#define IREAD/IWRITE/IEXEC	// Permissions : = VXXX かつ stat.hのS_IXUSRと同じ
	#define ITOV(ip), VTOI(vp)	// inodeとvnodeの相互変換
	enum de_op { DE_CREATE/MKDIR/LINK/RENAME }	// direnter ops
	enum dr_op { DR_REMOVE/RMDIR/RENAME }		// dirremove ops
	struct ufid {};			// struct fidを上書きして使用
	struct s5vfs {};		// VFSのS5用プライベートデータ
	#define S5VFS(vfsp)		// vspからstruct v5vspを取り出す
	#define IRWLOCK(ip), IRWUNLOCK(ip), ILOCK(ip), IUNLOCK(ip), ITIMES(ip)	// ロックと時刻変更を行うマクロ
```

## s5micros.h

```c
	#define FsXXX(fs, x)	// superblockのビット操作のためのマクロ
```

## s5param.h

```c
	#define 定数				// 定数パラメタ
```
	