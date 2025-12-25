#ifndef INC_PARAM_H
#define INC_PARAM_H

/* システム全体で使用する定数の定義 */
/* デバイス番号 */
#define DEVFAT          makedev(1, 0)
#define DEVV6           makedev(1, 1)
#define DEVCONS         makedev(2, 1)
#define DEVTTY1         makedev(3, 1)
#define DEVTTY2         makedev(3, 2)

/* vfs関係 */
#define FSTYPESIZE      16                  /* ファイルシステムタイプの最大値 */
#define NFILE           100                 /* システムあたりのオープンファイル数 */
#define NINODE          50                  /* 最大アクティブinode数 */
#define MAXOPBLOCKS     10                  /* ファイルシステム操作関数が1回に書き込める最大ブロック数 */
#define NBUF            (MAXOPBLOCKS*3)     /* ブロックキャッシュのサイズ */

#endif
