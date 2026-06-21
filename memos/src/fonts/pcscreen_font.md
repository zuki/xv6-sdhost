# [PC Screen Font](https://wiki.osdev.org/PC_Screen_Font)

すべてのLinuxディストリビューションにおいて、コンソールフォントは拡張子
`.psf`または`.psfu`を持っていることが確認できます。ターミナルで
`whereis consolefonts`と入力するとその場所を特定できます。本記事では
これらのフォントをグラフィカル画面に表示する方法を説明します。これには
フォントを修正する必要がなく、Linuxディストリビューションに同梱されている
フォントをそのまま使用できるとう利点があります。もう一つの利点は、
コンソールフォントは最大512グリフしか保持できないのに対し、PSFフォントは
Unicode文字セット全体を格納できることです。

PSFにはPSF 1とPSF 2の2つのバージョンがあります。各バージョンはマジック
ナンバーで判別できます。本記事ではPSF 2フォントを使用していることを
前提とします。

## ファイルの構造

PSFファイルはヘッダーとグリフ用のビットマップ、オプションのUnicode文字変換
テーブルで構成されます。

### ファイルヘッダー

ファイルの先頭にある固定チャンクです。フォントファイルの解析を進める前に
バージョンを検出する必要があります。

```c
#define PSF1_FONT_MAGIC 0x0436

/*
    PSF1ではグリフ幅は常に8ビットで、グリフ高はcharacterSizeです。
*/
typedef struct {
    uint16_t magic;         // 識別用のマジックバイト
    uint8_t fontMode;       // PSFフォントモード
    uint8_t characterSize;  // PSF文字サイズ
} PSF1_Header;


#define PSF_FONT_MAGIC 0x864ab572

/*
    PSF2
*/
typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;
```

### グリフ

各グリフはビットマップであり、[VGAフォント](https://wiki.osdev.org/VGA_Fonts)と
同じ方法でエンコードされます。8x16フォントの場合、各グリフは16バイト長であり、
各バイトはグリフの1行をエンコードします。

```bash
00000000b  byte  0
00000000b  byte  1
00000000b  byte  2
00010000b  byte  3
00111000b  byte  4
01101100b  byte  5
11000110b  byte  6
11000110b  byte  7
11111110b  byte  8
11000110b  byte  9
11000110b  byte 10
11000110b  byte 11
11000110b  byte 12
00000000b  byte 13
00000000b  byte 14
00000000b  byte 15
```

幅が8で割り切れないグリフについては、行をバイト境界で割り切れるように
パディングビットが追加されます。以下のpsfグリフ（O）は12x12です。
[PC Screen Font - Wikipedia](https://ja.wikipedia.org/wiki/PC_Screen_Font#%E3%83%95%E3%82%A9%E3%83%B3%E3%83%88%E3%83%95%E3%83%A9%E3%82%B0) から
着想を得ました。12は8で割り切れないため、各行を1バイトで割り切れるように
4ビットのパディングが追加されています。

```bash
 Glyph Data  padding
|-----------|----|
111111111111 0000
111111111111 0000
110000000011 0000
110000000011 0000
110000000011 0000
110000000011 0000
110000000011 0000
110000000011 0000
110000000011 0000
110000000011 0000
111111111111 0000
111111111111 0000
```

### Unicodeテーブル

PSFヘッダーのフラグが1の場合、そのフォントはグリフマッピング用のUnicode
テーブルを持っていることを示します。このテーブルがない場合、Unicode文字と
グリフは1対1にマッピングされます。すなわち、最初のグリフはUnicode文字0に
対応し、2番目のグリフはUnicode文字1に対応する、というように続きます。

テーブルの構造は次の通りです: 各グリフは可変長のレコードを持ちます。
これらはテキストファイルの行と非常に似ていますが、行末が '\n' (0x0A) では
なく 0xFF 文字で終了します。n 行目は n 番目のグリフのマッピングを記述します。
各行には少なくとも 1つ、場合によっては複数の UTF-8 文字列が含まれます。

## PSFの取り扱い

PSFエディタは数多く存在しますが、そのほとんどは不具合があったり使いにく
かったりします。そのため代わりに2つのPerlスクリプト: [readpsf](https://github.com/talamus/solarize-12x29-psf/)と
[writepsf](https://github.com/talamus/solarize-12x29-psf/)の使用を勧めます。
これらのスクリプトはPSFを編集しやすいASCIIテキストファイルやすべての画像編集
プログラムが開くことができるビットマップ画像に変換することができます。

## フォントの読み込み

VGAフォントで説明したように、いくつかの選択肢があります。簡便さを考慮し、
この例ではフォントをカーネル実行ファイルに埋め込みます。PSFをカーネル実行
ファイルにリンク可能なELFに変換する方法は以下の通りです。

```c
objcopy -O elf64-x86-64 -B i386 -I binary font.psf font.o
readelf -s font.o

Symbol table '.symtab' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1
     2: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_start
     3: 0000000000008020     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_end
     4: 0000000000008020     0 NOTYPE  GLOBAL DEFAULT  ABS _binary_font_psf_size
```

生成されたオブジェクトは3つのシンボルをエクスポートします。これらは他の変数と
同じように参照することができます（たとえば、C言語では`extern`を使用します）。

フォントを使用するにはまずUnicodeテーブルをデコードする必要があります。
これはトリッキーであり、動的メモリ割り当て（ここでは`calloc()`）が必要です。
ただし、「最初のグリフは文字0用、2番目のグリフは文字1用…」という方式で問題
なければこの処理は省略可能であることは朗報です。

```c
/* 上で作成されたオブジェクトファイルに埋め込まれたフォントをインポートする */
extern char _binary_font_psf_start;
extern char _binary_font_psf_end;

uint16_t *unicode;

void psf_init()
{
    uint16_t glyph = 0;
    /* アドレスをPSFヘッダー構造体にキャストする */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* unicodeテーブルは存在するか? */
    if (font->flags == 0) {
        unicode = NULL;
        return;
    }

    /* テーブルのオフセットを計算する */
    char *s = (char *)(
    (unsigned char*)&_binary_font_psf_start +
      font->headersize +
      font->numglyph * font->bytesperglyph
    );
    /* 翻訳テーブル用のメモリを割り当てる */
    unicode = calloc(USHRT_MAX, 2);
    while(s < (unsigned char*)&_binary_font_psf_end) {
        uint16_t uc = (uint16_t)((unsigned char *)s[0]);
        if (uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if (uc & 128) {
            /* UTF-8 to unicode */
            if ((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else if ((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else if ((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        unicode[uc] = glyph;
        s++;
    }
}
```

## 文字の表示

ここではリニアフレームバッファが適切に設定されており、ピクセルの
描画が可能であることを前提としています。以下の例では32ビットRGBA形式を
使用していますが、他の形式にも容易に適用できます。

```c
/* リニアフレームバッファ */
extern char *fb;
/* 各行のバイト数。画面幅 * bytesperpixel とは異なる場合がある。 */
extern int scanline;
/* 上で作成したオブジェクトファイルに組み込まれているフォントを取り込む */
extern char _binary_font_start[];

#define PIXEL uint32_t   /* ピクセルポインタ */

void putchar(
    /* unicode文字なのでcharではなくintであることに注意 */
    unsigned short int c,
    /* 画面上のカーソル位置。ピクセル単位ではなく文字単位 */
    int cx, int cy,
    /* 前面色と背景色、たとえば、0xFFFFFF と 0x000000 */
    uint32_t fg, uint32_t bg)
{
    /* アドレスをPSFヘッダー構造体にキャスト */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* unicode変換の有無 */
    if (unicode != NULL) {
        c = unicode[c];
    }
    /* 文字のグリフを取得する。指定の文字のグリフが存在しない場合は
       最初のグリフを表示する */
    unsigned char *glyph = (unsigned char*)&_binary_font_psf_start +
        font->headersize + (c > 0 && c < font->numglyph ? c : 0) * font->bytesperglyph;
    /* 表示する位置である画面の左上角を計算する。これは1回だけ行い、
       以後はオフセットを調整する。その方が早い。 */
    int offs =
        (cy * font->height * scanline) +
        (cx * (font->width + 1) * sizeof(PIXEL));

    /* 1つのグリフの1行に必要なバイト数を計算する。グリフ幅が
       バイトアラインにない場合は丸めあげる */
    uint32_t bytesPerGlyphLine = (width + 7) / 8;
    /* bitmapに従いピクセルを表示する */
    int x, y, line
    for(y = 0; y < font->height; y++){
        /* 行の開始位置を保存する */
        line = offs;
        /* グリフの最初の行の最初のバイトを計算する */
        unsigned char* currentByte = glyph + (bytesPerGlyphLine * y);
        /* このバイトの最上位ビットからマスクを開始する */
        uint8_t mask = 1<< 7;
        /* 1行表示 */
        for(x = 0; x <font->width; x++){
            *((PIXEL*)(fb + line)) = (*currentByte & mask) ? fg : bg;
            mask >>= 1;
            if (mask == 0){
                /* グリフのこのバイトは読み終わったので
                マスクをリセットして次のバイトに移動する */
                mask = 1<<7;
                currentByte += 1;
            }
            /* フレームバッファ内の次のピクセルに調整する */
            line += sizeof(PIXEL);
        }
        /* フレームバッファ内の次の行に調整する */
        offs  += scanline;
    }
}
```

このコードにより文字列をリニアフレームバッファ上に表示することが
できます。なお、このコードは最適化されたものではなく、あくまで
デモ用ですが、良い出発点となるでしょう。

オフセットの計算には画面上のグリフ間に1ピクセルの間隔を保つために
幅に1を加える必要があります。これを行わないと、一部のフォントが判読
不能になってしまいます。これはオリジナルの[VGAハードウェア](https://wiki.osdev.org/VGA_Hardware)の
動作と同じです。8x16のフォントを使用しますが実際には9x16として表示して
いました。最大32ピクセルのフォント幅に対応するためにマスクとの`AND`演算を
行う前にグリフをキャストする必要があります。

幅が8ピクセルの場合はunsigned charが必要ですが、9ピクセルの場合は
unsigned shortが必要になります。こうすることで、コンパイル時に必要な
サイズが不明な場合（読み込まれたフォントに依存するため）でも同じコードを
使用することができます。

`PIXEL`やlineを配列に置き換えたいと思うかもしれませんが、`scanline`は
バイト単位で指定されるため、すべてのハードウェアで動作するとは限りません。
また、`scanlline`が`PIXEL`の倍数でなければならないという規定は実際には
ありません（可能性は極めて低いですが、あり得ます）。

## 参考資料

- [VGAフォント](https://wiki.osdev.org/VGA_Fonts)
- [スケーラブルスクリーンフォント](https://wiki.osdev.org/Scalable_Screen_Font):
  フリーな小さなANSI Cレンダリングライブラリが付属
- [アイコンのローディング](https://wiki.osdev.org/Loading_Icons)

## 外部リンク

- [http://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html](http://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html) PC Screen Fontフォーマット解説
- [https://en.wikipedia.org/wiki/PC_Screen_Font](https://en.wikipedia.org/wiki/PC_Screen_Font) WikipediaのPC Screen Fontの項
- [https://github.com/talamus/solarize-12x29-psf/blob/master/readpsf](https://github.com/talamus/solarize-12x29-psf/blob/master/readpsf) PSFをtxtとbmpに変換するPerlスクリプト
- [https://github.com/talamus/solarize-12x29-psf/blob/master/writepsf](https://github.com/talamus/solarize-12x29-psf/blob/master/writepsf) txtとbmpをPSFに変換するPerlスクリプト
