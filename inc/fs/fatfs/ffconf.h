/*---------------------------------------------------------------------------/
/  FatFsの機能上の構成
/---------------------------------------------------------------------------*/

#define FFCONF_DEF	86631	/* Revision ID */

/*---------------------------------------------------------------------------/
/ 関数に関する構成
/---------------------------------------------------------------------------*/

#define FF_FS_READONLY	0
/* このオプションは読み取り専用設定を切り替えます（0：読み書き可能、1：読み取り専用）。
   読み取り専用設定にすると、書き込み用のAPI関数（f_write()、f_sync()、f_unlink()、
   f_mkdir()、f_chmod()、f_rename()、f_truncate()、f_getfree()）とオプションの
   書き込み関数が削除されます。*/
/* This option switches read-only configuration. (0:Read/Write or 1:Read-only)
/  Read-only configuration removes writing API functions, f_write(), f_sync(),
/  f_unlink(), f_mkdir(), f_chmod(), f_rename(), f_truncate(), f_getfree()
/  and optional writing functions as well. */


#define FF_FS_MINIMIZE	0
/* このオプションは一部の基本的なAPI関数を削除するための最小化レベルを定義します。
/
/   0: 基本関数はすべて有効です。
/   1: f_stat()、f_getfree()、f_unlink()、f_mkdir()、f_truncate()、
/      f_rename() が削除されます。
/   2: 1に加え、f_opendir()、f_readdir()、f_closedir() が削除されます。
/   3: 2に加え、f_lseek() 関数が削除されます。 */
/* This option defines minimization level to remove some basic API functions.
/
/   0: Basic functions are fully enabled.
/   1: f_stat(), f_getfree(), f_unlink(), f_mkdir(), f_truncate() and f_rename()
/      are removed.
/   2: f_opendir(), f_readdir() and f_closedir() are removed in addition to 1.
/   3: f_lseek() function is removed in addition to 2. */


#define FF_USE_FIND		0
/* このオプションはフィルタリングされたディレクトリ読み取り関数である f_findfirst()、
/  f_findnext() の動作を切り替えます。(0:無効、1:有効、2:altname[] との一致も有効) */
/* This option switches filtered directory read functions, f_findfirst() and
/  f_findnext(). (0:Disable, 1:Enable 2:Enable with matching altname[] too) */


#define FF_USE_MKFS		0
/* このオプションは f_mkfs() 関数の動作を切り替えます。(0:無効、1:有効) */
/* This option switches f_mkfs() function. (0:Disable or 1:Enable) */


#define FF_USE_FASTSEEK	0
/* このオプションは高速シーク関数の動作を切り替えます。（0：無効、1：有効） */
/* This option switches fast seek function. (0:Disable or 1:Enable) */


#define FF_USE_EXPAND	0
/* このオプションはf_expand関数の動作を切り替えます。（0：無効、1：有効） */
/* This option switches f_expand function. (0:Disable or 1:Enable) */

// TODO: 要検討
#define FF_USE_CHMOD	1
/* このオプションは属性操作関数である f_chmod()、f_utime() の動作を切り替えます
/  (0:無効、1:有効)。このオプションを有効にするには、FF_FS_READONLY を 0 に設定する必要があります。*/
/* This option switches attribute manipulation functions, f_chmod() and f_utime().
/  (0:Disable or 1:Enable) Also FF_FS_READONLY needs to be 0 to enable this option. */


#define FF_USE_LABEL	0
/* このオプションはボリュームラベル関数である f_getlabel()、f_setlabel() の動作を切り替えます
/ （0：無効、1：有効）*/
/* This option switches volume label functions, f_getlabel() and f_setlabel().
/  (0:Disable or 1:Enable) */


#define FF_USE_FORWARD	0
/* このオプションはf_forward()関数の動作を切り替えます。（0：無効、1：有効） */
/* This option switches f_forward() function. (0:Disable or 1:Enable) */


#define FF_USE_STRFUNC	0
#define FF_PRINT_LLI	0
#define FF_PRINT_FLOAT	0
#define FF_STRF_ENCODE	0
/* FF_USE_STRFUNC は、文字列関数 f_gets()、f_putc()、f_puts()、f_printf() を切り替えます。
/
/ 0: 無効。FF_PRINT_LLI、FF_PRINT_FLOAT、FF_STRF_ENCODE は効果を持ちません。
/ 1: LF-CRLF変換なしで有効にします。
/ 2: LF-CRLF変換ありで有効にします。
/
/ FF_PRINT_LLI = 1 にすると f_printf() が long long 引数をサポートし、FF_PRINT_FLOAT = 1/2 に
/ すると f_printf() が浮動小数点引数をサポートします。これらの機能には C99 以降が必要です。
/ LFN が有効な状態で FF_LFN_UNICODE >= 1 の場合、文字列関数はその中の文字エンコーディングを変換します。
/ FF_STRF_ENCODE は、これらの関数を通じて読み書きされるファイルの想定される文字エンコーディングを選択します。
/
/ 0: 現在のCPにおけるANSI-EM
/ 1: UTF-16LE形式のUnicode
/ 2: UTF-16BE形式のUnicode
/ 3: UTF-8形式のUnicode
*/
/* FF_USE_STRFUNC switches string functions, f_gets(), f_putc(), f_puts() and
/  f_printf().
/
/   0: Disable. FF_PRINT_LLI, FF_PRINT_FLOAT and FF_STRF_ENCODE have no effect.
/   1: Enable without LF-CRLF conversion.
/   2: Enable with LF-CRLF conversion.
/
/  FF_PRINT_LLI = 1 makes f_printf() support long long argument and FF_PRINT_FLOAT = 1/2
   makes f_printf() support floating point argument. These features want C99 or later.
/  When FF_LFN_UNICODE >= 1 with LFN enabled, string functions convert the character
/  encoding in it. FF_STRF_ENCODE selects assumption of character encoding ON THE FILE
/  to be read/written via those functions.
/
/   0: ANSI/OEM in current CP
/   1: Unicode in UTF-16LE
/   2: Unicode in UTF-16BE
/   3: Unicode in UTF-8
*/


/*---------------------------------------------------------------------------/
/ ロケールと名前空間に関する構成
/---------------------------------------------------------------------------*/
#define FF_CODE_PAGE	437

/* このオプションはターゲットシステムで使用するOEMコードページを指定します。
/  コードページの設定が正しくない場合、ファイルを開く際にエラーが発生する可能性があります。*/
/* This option specifies the OEM code page to be used on the target system.
/  Incorrect code page setting can cause a file open failure.
/
/   437 - U.S.
/   720 - Arabic
/   737 - Greek
/   771 - KBL
/   775 - Baltic
/   850 - Latin 1
/   852 - Latin 2
/   855 - Cyrillic
/   857 - Turkish
/   860 - Portuguese
/   861 - Icelandic
/   862 - Hebrew
/   863 - Canadian French
/   864 - Arabic
/   865 - Nordic
/   866 - Russian
/   869 - Greek 2
/   932 - Japanese (DBCS)
/   936 - Simplified Chinese (DBCS)
/   949 - Korean (DBCS)
/   950 - Traditional Chinese (DBCS)
/     0 - Include all code pages above and configured by f_setcp()
*/

// fxl: FF_USE_LFN=0 only 8.3 format filename is supported. longer filenames -> API failure
// too limited.
#define FF_USE_LFN	2		/* 0 - 3 */
#define FF_MAX_LFN	64		/* 処理する最長LFN長 (12 - 255) */

/* FF_USE_LFN は、LFN（長いファイル名）のサポートを切り替えます。
/
/ 0: LFN を無効にします。FF_MAX_LFN は無効になります。
/ 1: BSS 上の静的作業バッファを使用して LFN を有効にします。常にスレッドセーフではありません。
/ 2: STACK 上の動的作業バッファを使用して LFN を有効にします。
/ 3: HEAP　上の動的な作業バッファを使用してLFNを有効にします。
/
/ LFNを有効にするには、プロジェクトにffunicode.cを追加する必要があります。LFN関数は
/ 内部作業バッファとして(FF_MAX_LFN + 1) * 2バイトを占有し、exFATが有効な場合は
/ さらに(FF_MAX_LFN + 44) * 15 * 32バイトを必要とします。
/ FF_MAX_LFN は、UTF-16 コードユニット単位での作業バッファのサイズを定義し、12 から 255 の
/ 範囲で指定できます。LFN 仕様を完全にサポートするには、255 に設定することを推奨します。
/ ワーキングバッファにスタックを使用する場合は、スタックオーバーフローに注意してください。
/ ワーキングバッファにヒープメモリを使用する場合は、ffsystem.c に例示されているメモリ
/ 管理関数 ff_memalloc() と ff_memfree() をプロジェクトに追加する必要があります。 */

/* The FF_USE_LFN switches the support for LFN (long file name).
/
/   0: Disable LFN. FF_MAX_LFN has no effect.
/   1: Enable LFN with static  working buffer on the BSS. Always NOT thread-safe.
/   2: Enable LFN with dynamic working buffer on the STACK.
/   3: Enable LFN with dynamic working buffer on the HEAP.
/
/  To enable the LFN, ffunicode.c needs to be added to the project. The LFN function
/  requiers certain internal working buffer occupies (FF_MAX_LFN + 1) * 2 bytes and
/  additional (FF_MAX_LFN + 44) / 15 * 32 bytes when exFAT is enabled.
/  The FF_MAX_LFN defines size of the working buffer in UTF-16 code unit and it can
/  be in range of 12 to 255. It is recommended to be set it 255 to fully support LFN
/  specification.
/  When use stack for the working buffer, take care on stack overflow. When use heap
/  memory for the working buffer, memory management functions, ff_memalloc() and
/  ff_memfree() exemplified in ffsystem.c, need to be added to the project. */


#define	FF_LFN_UNICODE	0	/* 0:ANSI/OEM or 1:Unicode */
/* このオプションはLFNが有効な場合にAPIの文字エンコーディングを切り替えます。
/
/ 0: 現在のCPでのANSI/OEN（TCHAR = char）
/ 1: UTF-16形式のUnicode（TCHAR = WCHAR）
/ 2: UTF-8形式のUnicode（TCHAR = char）
/ 3: UTF-32形式のUnicode（TCHAR = DWORD）
/
/ また、このオプションは文字列入出力関数の動作にも影響します。
/ LFNが有効になっていない場合、このオプションは効果を持ちません。*/
/* This option switches the character encoding on the API when LFN is enabled.
/
/   0: ANSI/OEM in current CP (TCHAR = char)
/   1: Unicode in UTF-16 (TCHAR = WCHAR)
/   2: Unicode in UTF-8 (TCHAR = char)
/   3: Unicode in UTF-32 (TCHAR = DWORD)
/
/  Also behavior of string I/O functions will be affected by this option.
/  When LFN is not enabled, this option has no effect. */

#define FF_LFN_BUF		255
#define FF_SFN_BUF		12
/* この２つのオプションはディレクトリ項目を読み出す際に使用されるFILINFO構造体内の
/  ファイル名メンバのサイズを定義します。これらの値は、読み出すファイル名に対して
/  十分な長さである必要があります。読み出されるファイル名の最大長は文字エンコーディングに
/  よって異なります。LFNが有効になっていない場合、これらのオプションは効果を持ちません。 */
/* This set of options defines size of file name members in the FILINFO structure
/  which is used to read out directory items. These values should be suffcient for
/  the file names to read. The maximum possible length of the read file name depends
/  on character encoding. When LFN is not enabled, these options have no effect. */

#define FF_FS_RPATH		2     // fxl
/* このオプションは、相対パスのサポートを設定します。
/
/  0: 相対パスを無効にし、関連する関数を削除します。
/  1: 相対パスを有効にします。f_chdir() と f_chdrive() が利用可能になります。
/  2: 1 に加えて、f_getcwd() 関数が利用可能になります。*/
/* This option configures support for relative path.
/
/   0: Disable relative path and remove related functions.
/   1: Enable relative path. f_chdir() and f_chdrive() are available.
/   2: f_getcwd() function is available in addition to 1.
*/

/*---------------------------------------------------------------------------/
/ ドライブ/ボリューム関係の構成
/---------------------------------------------------------------------------*/

#define FF_VOLUMES	10
/* 使用するボリューム（論理ドライブ）の数。（1-10）*/
/* Number of volumes (logical drives) to be used. (1-10) */

#define FF_STR_VOLUME_ID	0
#define FF_VOLUME_STRS		"RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
/* FF_STR_VOLUME_ID は任意の文字列によるボリューム ID のサポートを有効にします。
/  FF_STR_VOLUME_ID に 1 または 2 を設定するとパス名内のドライブ番号として任意の
/  文字列を使用できます。FF_VOLUME_STRS は、各論理ドライブのボリューム ID 文字列を定義します。
/  項目数は FF_VOLUMES 以上でなければなりません。ボリューム ID 文字列に使用できる文字は
/  A-Z、a-z、0-9 ですが、大文字と小文字は区別されません。FF_STR_VOLUME_ID >= 1 であり、
/  かつ FF_VOLUME_STRS が定義されていない場合、ユーザー定義のボリューム文字列テーブルを
/  次のように定義する必要があります.
/
/  const char* VolumeStr[FF_VOLUMES] = {"ram","flash","sd","usb",...
*/
/* FF_STR_VOLUME_ID switches support for volume ID in arbitrary strings.
/  When FF_STR_VOLUME_ID is set to 1 or 2, arbitrary strings can be used as drive
/  number in the path name. FF_VOLUME_STRS defines the volume ID strings for each
/  logical drives. Number of items must not be less than FF_VOLUMES. Valid
/  characters for the volume ID strings are A-Z, a-z and 0-9, however, they are
/  compared in case-insensitive. If FF_STR_VOLUME_ID >= 1 and FF_VOLUME_STRS is
/  not defined, a user defined volume string table needs to be defined as:
/
/  const char* VolumeStr[FF_VOLUMES] = {"ram","flash","sd","usb",...
*/


#define FF_MULTI_PARTITION	0
/* このオプションは物理ドライブ上のマルチボリュームに対するサポートを切り替えます。
/  デフォルト（0）では、各論理ドライブ番号は同じ物理ドライブ番号に紐付けられ、
/  その物理ドライブ上に存在するFATボリュームのみがマウントされます。
/  この機能を有効にする（1）と、各論理ドライブ番号を、VolToPart[]にリストされている
/  任意の物理ドライブとパーティションに紐付けることができます。また、f_fdisk() 関数も
/  利用可能になります。 */
/* This option switches support for multiple volumes on the physical drive.
/  By default (0), each logical drive number is bound to the same physical drive
/  number and only an FAT volume found on the physical drive will be mounted.
/  When this function is enabled (1), each logical drive number can be bound to
/  arbitrary physical drive and partition listed in the VolToPart[]. Also f_fdisk()
/  funciton will be available. */


#define FF_MIN_SS		512
#define	FF_MAX_SS		512		/* 512, 1024, 2048 or 4096 */
/* この2つのオプションはサポートするセクタサイズの範囲を設定します（512、1024、2048、または4096）。
/  ほとんどのシステム、一般的なメモリカード、およびハードディスクでは常に両方を512に設定しますが、
/  オンボードフラッシュメモリや一部の光学メディアでは、より大きな値が必要になる場合があります。
/  FF_MAX_SS が FF_MIN_SS より大きい場合、FatFs は可変セクタサイズモードに設定され、
/  disk_ioctl() 関数では GET_SECTOR_SIZE コマンドを実装する必要があります。 */
/* This set of options configures the range of sector size to be supported. (512,
/  1024, 2048 or 4096) Always set both 512 for most systems, generic memory card and
/  harddisk, but a larger value may be required for on-board flash memory and some
/  type of optical media. When FF_MAX_SS is larger than FF_MIN_SS, FatFs is configured
/  for variable sector size mode and disk_ioctl() function needs to implement
/  GET_SECTOR_SIZE command. */


#define FF_LBA64		0
/* このオプションは64ビットLBAのサポートを切り替えます。（0：無効、1：有効）
/  64ビットLBAを有効にするには、exFATも有効にする必要があります。 (FF_FS_EXFAT == 1) */
/* This option switches support for 64-bit LBA. (0:Disable or 1:Enable)
/  To enable the 64-bit LBA, also exFAT needs to be enabled. (FF_FS_EXFAT == 1) */

#define FF_MIN_GPT		0x10000000
/* f_mkfs と f_fdisk 関数において、パーティション形式を GPT に変更するための
/  最小セクタ数。最大値は 0x100000000 です。FF_LBA64 == 0 の場合、このオプションは効果を持ちません。*/
/* Minimum number of sectors to switch GPT as partitioning format in f_mkfs and
/  f_fdisk function. 0x100000000 max. This option has no effect when FF_LBA64 == 0. */

#define FF_USE_TRIM		0
/* このオプションはATA-TRIMのサポートを切り替えます。（0：無効、1：有効）
/  Trim機能を有効にするには、disk_ioctl()関数にCTRL_TRIMコマンドも実装する必要があります。*/
/* This option switches support for ATA-TRIM. (0:Disable or 1:Enable)
/  To enable Trim function, also CTRL_TRIM command should be implemented to the
/  disk_ioctl() function. */


/*---------------------------------------------------------------------------/
/ システムに関する構成
/---------------------------------------------------------------------------*/

#define FF_FS_TINY		0
/* このオプションはTinyバッファ構成を切り替えます（0：通常、1：Tiny）。
/  Tiny構成では、ファイルオブジェクト（FIL）のサイズがFF_MAX_SSバイト縮小されます。
/  ファイルオブジェクトからプライベートセクタバッファが削除される代わりに、
/  ファイルシステムオブジェクト（FATFS）内の共通セクタバッファがファイルデータの転送に使用されます。 */
/* This option switches tiny buffer configuration. (0:Normal or 1:Tiny)
/  At the tiny configuration, size of file object (FIL) is shrinked FF_MAX_SS bytes.
/  Instead of private sector buffer eliminated from the file object, common sector
/  buffer in the filesystem object (FATFS) is used for the file data transfer. */

#define FF_FS_EXFAT	0        // fxl: consider
/* このオプションはexFATファイルシステムのサポートを切り替えます。（0：無効、1：有効）
/  exFATを有効にするには、LFNも有効にする必要があります。（FF_USE_LFN >= 1）
/  exFATを有効にすると、ANSI C（C89）との互換性が失われる点に注意してください。 */
/* This option switches support for exFAT filesystem. (0:Disable or 1:Enable)
/  To enable exFAT, also LFN needs to be enabled. (FF_USE_LFN >= 1)
/  Note that enabling exFAT discards ANSI C (C89) compatibility. */

// TODO: 要検討
#define FF_FS_NORTC		1
#define FF_NORTC_MON	1
#define FF_NORTC_MDAY	1
#define FF_NORTC_YEAR	2024
/* オプション FF_FS_NORTC は、タイムスタンプ機能を切り替えます。システムに RTC 機能が
/ ない場合や、有効なタイムスタンプが必要ない場合は、FF_FS_NORTC = 1 に設定して
/ タイムスタンプ機能を無効にしてください。FatFs によって変更されるすべてのオブジェクトには
/ FF_NORTC_MON、FF_NORTC_MDAY、FF_NORTC_YEAR で定義されたローカルタイムの
/ 固定タイムスタンプが付与されます。
/ タイムスタンプ機能を有効にする場合（FF_FS_NORTC = 0）、リアルタイムクロックから
/ 現在の時刻を読み取るために、プロジェクトに get_fattime() 関数を追加する必要があります。
/ この場合、FF_FS_NORTC_MON、FF_FS_NORTC_MDAY、FF_FS_NORTC_YEAR は無効になります。
/ これらのオプションは、読み取り専用構成（FF_FS_READONLY = 1）では無効です。
*/
/* The option FF_FS_NORTC switches timestamp functiton. If the system does not have
/  any RTC function or valid timestamp is not needed, set FF_FS_NORTC = 1 to disable
/  the timestamp function. Every object modified by FatFs will have a fixed timestamp
/  defined by FF_NORTC_MON, FF_NORTC_MDAY and FF_NORTC_YEAR in local time.
/  To enable timestamp function (FF_FS_NORTC = 0), get_fattime() function need to be
/  added to the project to read current time form real-time clock. FF_NORTC_MON,
/  FF_NORTC_MDAY and FF_NORTC_YEAR have no effect.
/  These options have no effect in read-only configuration (FF_FS_READONLY = 1). */


#define FF_FS_NOFSINFO	0
/* FAT32ボリュームの正確な空き領域を知る必要がある場合はこのオプションのビット0を設定してください。
/  そうすると、ボリュームのマウント直後にf_getfree()関数がFATの完全スキャンを強制的に実行します。
/  ビット1は、最後に割り当てられたクラスタ番号の使用を制御します。
/
/ bit0=0: 利用可能な場合、FSINFOの空きクラスタ数を使用します。
/ bit0=1: FSINFOの空きクラスタ数を信頼しない。
/ bit1=0: 利用可能な場合、FSINFOの最後に割り当てられたクラスタ番号を使用します。
/ bit1=1: FSINFOの最後に割り当てられたクラスタ番号を信頼しない。
*/
/* If you need to know correct free space on the FAT32 volume, set bit 0 of this
/  option, and f_getfree() function at first time after volume mount will force
/  a full FAT scan. Bit 1 controls the use of last allocated cluster number.
/
/  bit0=0: Use free cluster count in the FSINFO if available.
/  bit0=1: Do not trust free cluster count in the FSINFO.
/  bit1=0: Use last allocated cluster number in the FSINFO if available.
/  bit1=1: Do not trust last allocated cluster number in the FSINFO.
*/

// TODO: 要検討
#define FF_FS_LOCK		0
/* オプション FF_FS_LOCK はファイルの重複オープンやオブジェクトに対する不正な操作を
/  制御するためのファイルロック機能を切り替えます。FF_FS_READONLY が 1 の場合、
/  このオプションは 0 でなければなりません。
/
/  0:  ファイルロック機能を無効にします。ボリュームの破損を防ぐため、アプリケーション
/      プログラムは、開いているオブジェクトに対する不正なオープン、削除、および名前変更を
/      避ける必要があります。
/  >0: ファイルロック機能を有効にします。この値は、ファイルロック制御下で同時に開くことが
/      できるファイル/サブディレクトリの数を定義します。ファイルロック制御は再入可能性とは
/      独立していることに注意してください。
*/
/* The option FF_FS_LOCK switches file lock function to control duplicated file open
/  and illegal operation to open objects. This option must be 0 when FF_FS_READONLY
/  is 1.
/
/  0:  Disable file lock function. To avoid volume corruption, application program
/      should avoid illegal open, remove and rename to the open objects.
/  >0: Enable file lock function. The value defines how many files/sub-directories
/      can be opened simultaneously under file lock control. Note that the file
/      lock control is independent of re-entrancy. */


/* #include <somertos.h>	// O/S definitions */

// fxl: our kernel has own sync
#define FF_FS_REENTRANT	0		/* 0:Disable or 1:Enable */
// #define FF_FS_TIMEOUT	RT_DFS_ELM_MUTEX_TIMEOUT
// #define FF_SYNC_t		rt_mutex_t
/* オプション FF_FS_REENTRANT は FatFs モジュール自体の再入可能性（スレッドセーフ）を
/  切り替えます。このオプションの設定にかかわらず、異なるボリュームへのファイルアクセスは
/  常に再入可能であり、ボリューム制御関数である f_mount()、f_mkfs()、f_fdisk() は
/  常に再入可能でないことに注意してください。このオプションが制御するのは、同一ボリュームへの
/  ファイル/ディレクトリへのアクセスのみです。（訳注: ボリューム全体に対するジャンボlock）
/
/ 0: 再入可能性を無効にします。FF_FS_TIMEOUT と FF_SYNC_t は無効になります。
/ 1: 再入可能性を有効にします。また、ユーザが提供する同期ハンドラ ff_req_grant()、
/    ff_rel_grant()、ff_del_syncobj()、ff_cre_syncobj() 関数をプロジェクトに追加する
/    必要があります。サンプルは optionsyscall.c に用意されています。
/
/  FF_FS_TIMEOUT はタイムアウト期間をタイムティック単位で定義します。
/  FF_SYNC_t はOS 依存の同期オブジェクト型を定義します。たとえば、HANDLE、ID、OS_EVENT*、
/  SemaphoreHandle_t など。OS 定義用のヘッダーファイルをff.h のスコープ内のどこかに
/  インクルードする必要があります。 */
/* The option FF_FS_REENTRANT switches the re-entrancy (thread safe) of the FatFs
/  module itself. Note that regardless of this option, file access to different
/  volume is always re-entrant and volume control functions, f_mount(), f_mkfs()
/  and f_fdisk() function, are always not re-entrant. Only file/directory access
/  to the same volume is under control of this function.
/
/   0: Disable re-entrancy. FF_FS_TIMEOUT and FF_SYNC_t have no effect.
/   1: Enable re-entrancy. Also user provided synchronization handlers,
/      ff_req_grant(), ff_rel_grant(), ff_del_syncobj() and ff_cre_syncobj()
/      function, must be added to the project. Samples are available in
/      option/syscall.c.
/
/  The FF_FS_TIMEOUT defines timeout period in unit of time tick.
/  The FF_SYNC_t defines O/S dependent sync object type. e.g. HANDLE, ID, OS_EVENT*,
/  SemaphoreHandle_t and etc. A header file for O/S definitions needs to be
/  included somewhere in the scope of ff.h. */

/*--- End of configuration options ---*/
