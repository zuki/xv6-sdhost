# PSF v1のunicodeテーブル

- PSF v1のフォントかunicodeテーブルを抽出する
- 例としてUbuntuMono-B-8x16.psfをfont.psfにrenameして使用

```bash
$ file UbuntuMono-B-8x16.psf
UbuntuMono-B-8x16.psf: Linux/i386 PC Screen Font v1 data, 256 characters, Unicode directory, 8x16
$ cp UbuntuMono-B-8x16.psf font.psf
$ xxd font.psf | head -2
00000000: 3604 0210 0000 0000 0010 1824 766e 3c28  6..........$vn<(
00000010: 1000 0000 2bbf 18de 93c9 b15e dfbe 725a  ....+......^..rZ

typedef struct {
    uint16_t magic;         // 0x3604
    uint8_t fontMode;       // 0x02
    uint8_t characterSize;  // 0x10
} PSF1_Header;

$ objcopy -O elf64-x86-64 -B i386 -I binary font.psf font.o
$ readelf -sW font.o
Symbol table '.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_start
     2: 00000000000013b2     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_end
     3: 00000000000013b2     0 NOTYPE  GLOBAL DEFAULT  ABS _binary_font_psf_size
```

## font.o内のunicodeテーブルを確認

```bash
elf header : 0x40
address of unicode table s =
    _binary_font_psf_start + sizeof(struct PSF1_Heder)
    + font->characterSize * (font->fontMode & 0x01) ? 512 : 256;
s = 0 + 4 + 16 * 256 = 4100 (0x1004)

font_psf.oではunicode tableは以下の範囲にある
 0x1044 - 0x13f2
```

## uncodeテーブルの内容

```bash
00001040:           fdff ffff ffff ffff ffff ffff  ................
00001050: ffff ffff 2220 ffff ffff ffff ffff ffff  ...." ..........
00001060: ffff ffff ffff ffff ffff ffff ffff ffff  ................
00001070: b600 ffff a700 ffff ffff ffff ffff ffff  ................
00001080: ffff ffff ffff ffff ffff ffff 2000 ffff  ............ ...
00001090: 2100 ffff 2200 ffff 2300 ffff 2400 ffff  !..."...#...$...
000010a0: 2500 ffff 2600 ffff 2700 ffff 2800 ffff  %...&...'...(...
000010b0: 2900 ffff 2a00 ffff 2b00 ffff 2c00 ffff  )...*...+...,...
000010c0: 2d00 ffff 2e00 ffff 2f00 ffff 3000 ffff  -......./...0...
000010d0: 3100 ffff 3200 ffff 3300 ffff 3400 ffff  1...2...3...4...
000010e0: 3500 ffff 3600 ffff 3700 ffff 3800 ffff  5...6...7...8...
000010f0: 3900 ffff 3a00 ffff 3b00 ffff 3c00 ffff  9...:...;...<...
00001100: 3d00 ffff 3e00 ffff 3f00 ffff 4000 ffff  =...>...?...@...
00001110: 4100 ffff 4200 ffff 4300 ffff 4400 ffff  A...B...C...D...
00001120: 4500 ffff 4600 ffff 4700 ffff 4800 ffff  E...F...G...H...
00001130: 4900 ffff 4a00 ffff 4b00 ffff 4c00 ffff  I...J...K...L...
00001140: 4d00 ffff 4e00 ffff 4f00 ffff 5000 ffff  M...N...O...P...
00001150: 5100 ffff 5200 ffff 5300 ffff 5400 ffff  Q...R...S...T...
00001160: 5500 ffff 5600 ffff 5700 ffff 5800 ffff  U...V...W...X...
00001170: 5900 ffff 5a00 ffff 5b00 ffff 5c00 ffff  Y...Z...[...\...
00001180: 5d00 ffff 5e00 ffff 5f00 ffff 6000 ffff  ]...^..._...`...
00001190: 6100 ffff 6200 ffff 6300 ffff 6400 ffff  a...b...c...d...
000011a0: 6500 ffff 6600 ffff 6700 ffff 6800 ffff  e...f...g...h...
000011b0: 6900 ffff 6a00 ffff 6b00 ffff 6c00 ffff  i...j...k...l...
000011c0: 6d00 ffff 6e00 ffff 6f00 ffff 7000 ffff  m...n...o...p...
000011d0: 7100 ffff 7200 ffff 7300 ffff 7400 ffff  q...r...s...t...
000011e0: 7500 ffff 7600 ffff 7700 ffff 7800 ffff  u...v...w...x...
000011f0: 7900 ffff 7a00 ffff 7b00 ffff 7c00 ffff  y...z...{...|...
00001200: 7d00 ffff 7e00 ffff ffff c700 ffff fc00  }...~...........
00001210: ffff e900 ffff e200 ffff e400 ffff e000  ................
00001220: ffff e500 ffff e700 ffff ea00 ffff eb00  ................
00001230: ffff e800 ffff ef00 ffff ee00 ffff ec00  ................
00001240: ffff c400 ffff c500 ffff c900 ffff e600  ................
00001250: ffff c600 ffff f400 ffff f600 ffff f200  ................
00001260: ffff fb00 ffff f900 ffff ff00 ffff d600  ................
00001270: ffff dc00 ffff a200 ffff a300 ffff a500  ................
00001280: ffff ffff 9201 ffff e100 ffff ed00 ffff  ................
00001290: f300 ffff fa00 ffff f100 ffff d100 ffff  ................
000012a0: aa00 ffff ba00 ffff bf00 ffff ffff ac00  ................
000012b0: ffff bd00 ffff bc00 ffff a100 ffff ab00  ................
000012c0: ffff bb00 ffff 9125 ffff 9225 ffff 9325  .......%...%...%
000012d0: ffff 0225 ffff 2425 ffff 6125 ffff 6225  ...%..$%..a%..b%
000012e0: ffff 5625 ffff 5525 ffff 6325 ffff 5125  ..V%..U%..c%..Q%
000012f0: ffff 5725 ffff 5d25 ffff 5c25 ffff 5b25  ..W%..]%..\%..[%
00001300: ffff 1025 ffff 1425 ffff 3425 ffff 2c25  ...%...%..4%..,%
00001310: ffff 1c25 ffff 0025 ffff 3c25 ffff 5e25  ...%...%..<%..^%
00001320: ffff 5f25 ffff 5a25 ffff 5425 ffff 6925  .._%..Z%..T%..i%
00001330: ffff 6625 ffff 6025 ffff 5025 ffff 6c25  ..f%..`%..P%..l%
00001340: ffff 6725 ffff 6825 ffff 6425 ffff 6525  ..g%..h%..d%..e%
00001350: ffff 5925 ffff 5825 ffff 5225 ffff 5325  ..Y%..X%..R%..S%
00001360: ffff 6b25 ffff 6a25 ffff 1825 ffff 0c25  ..k%..j%...%...%
00001370: ffff 8825 ffff ffff ffff ffff ffff b103  ...%............
00001380: ffff df00 ffff 9303 ffff c003 ffff a303  ................
00001390: ffff c303 ffff b500 ffff c403 ffff a603  ................
000013a0: ffff 9803 ffff a903 ffff b403 ffff 1e22  ..............."
000013b0: ffff c603 ffff b503 ffff ffff ffff b100  ................
000013c0: ffff 6522 ffff 6422 ffff ffff ffff f700  ..e"..d"........
000013d0: ffff 4822 ffff b000 ffff 1922 ffff b700  ..H"......."....
000013e0: ffff 1a22 ffff ffff b200 ffff ffff a000  ..."............
000013f0: ffff
```

## 抽出コード

- OSDevの[PC Screen Font](https://wiki.osdev.org/PC_Screen_Font)をv1用に変更
- 0xfffe（サロゲートペアを表現?）は見当たらなかったのでとりあえず無視した

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

typedef struct {
    uint16_t magic; // Magic bytes for identification.
    uint8_t fontMode; // PSF font mode.
    uint8_t characterSize; // PSF character size.
} PSF1_Header;

/* import the font contained in the object file created above */
extern char _binary_font_psf_start;
extern char _binary_font_psf_end;

uint16_t *unicode;

void psf_init()
{
    uint16_t glyph = 0;
    /* cast the address to PSF header struct */
    PSF1_Header *font = (PSF1_Header *)&_binary_font_psf_start;
    /* is there a unicode table? */
     if ((font->fontMode & 0x02) == 0) {
        printf("no unicode\n");
        unicode = NULL;
        return;
    }

    int32_t numGlyph = font->fontMode & 0x01 ? 512 : 256;

    /* get the offset of the table */
    char *s = (char *)
       (unsigned char*)&_binary_font_psf_start +
       4 + numGlyph * font->characterSize;
    /* allocate memory for translation table */
    unicode = calloc(numGlyph, 2);
    while(s < (unsigned char*)&_binary_font_psf_end) {
        uint16_t uc = (uint16_t)((unsigned char *)s[0]);
        //printf("uc: 0x%x\n", uc);
        if(uc == 0xFFFF) {
            glyph++;
            s+=2;
            continue;
        }
        /* save translation */
        unicode[uc] = glyph;
        s+=2;
    }
}

int main(void) {
    psf_init();
    for (int i = 0; i < 126; i++)
    	printf("unicode[%d] = %d\n", i, unicode[i]);

    return 0;
}
```

## 処理結果のunicode変換表

```bash
unicode[0] = 197
unicode[1] = 0
unicode[2] = 180
unicode[3] = 0
unicode[4] = 0
unicode[5] = 0
unicode[6] = 0
unicode[7] = 0
unicode[8] = 0
unicode[9] = 0
unicode[10] = 0
unicode[11] = 0
unicode[12] = 219
unicode[13] = 0
unicode[14] = 0
unicode[15] = 0
unicode[16] = 192
unicode[17] = 0
unicode[18] = 0
unicode[19] = 0
unicode[20] = 193
unicode[21] = 0
unicode[22] = 0
unicode[23] = 0
unicode[24] = 218
unicode[25] = 250
unicode[26] = 252
unicode[27] = 0
unicode[28] = 196
unicode[29] = 0
unicode[30] = 237
unicode[31] = 0
unicode[32] = 32
unicode[33] = 33
unicode[34] = 34
unicode[35] = 35
unicode[36] = 181
unicode[37] = 37
unicode[38] = 38
unicode[39] = 39
unicode[40] = 40
unicode[41] = 41
unicode[42] = 42
unicode[43] = 43
unicode[44] = 195
unicode[45] = 45
unicode[46] = 46
unicode[47] = 47
unicode[48] = 48
unicode[49] = 49
unicode[50] = 50
unicode[51] = 51
unicode[52] = 194
unicode[53] = 53
unicode[54] = 54
unicode[55] = 55
unicode[56] = 56
unicode[57] = 57
unicode[58] = 58
unicode[59] = 59
unicode[60] = 198
unicode[61] = 61
unicode[62] = 62
unicode[63] = 63
unicode[64] = 64
unicode[65] = 65
unicode[66] = 66
unicode[67] = 67
unicode[68] = 68
unicode[69] = 69
unicode[70] = 70
unicode[71] = 71
unicode[72] = 248
unicode[73] = 73
unicode[74] = 74
unicode[75] = 75
unicode[76] = 76
unicode[77] = 77
unicode[78] = 78
unicode[79] = 79
unicode[80] = 206
unicode[81] = 187
unicode[82] = 214
unicode[83] = 215
unicode[84] = 202
unicode[85] = 185
unicode[86] = 184
unicode[87] = 188
unicode[88] = 213
unicode[89] = 212
unicode[90] = 201
unicode[91] = 191
unicode[92] = 190
unicode[93] = 189
unicode[94] = 199
unicode[95] = 200
unicode[96] = 205
unicode[97] = 182
unicode[98] = 183
unicode[99] = 186
unicode[100] = 244
unicode[101] = 243
unicode[102] = 204
unicode[103] = 208
unicode[104] = 209
unicode[105] = 203
unicode[106] = 217
unicode[107] = 216
unicode[108] = 207
unicode[109] = 109
unicode[110] = 110
unicode[111] = 111
unicode[112] = 112
unicode[113] = 113
unicode[114] = 114
unicode[115] = 115
unicode[116] = 116
unicode[117] = 117
unicode[118] = 118
unicode[119] = 119
unicode[120] = 120
unicode[121] = 121
unicode[122] = 122
unicode[123] = 123
unicode[124] = 124
unicode[125] = 125
```
