# PSF v2のunicodeテーブル

- PSF v2のフォントかunicodeテーブルを抽出する
- 例としてUbuntuMono-B-8x16.psfをv2に変換したフォントをfont.psfにrenameして使用

```bash
$ xxd font.psf | head -2
00000000: 72b5 4a86 0000 0000 2000 0000 0100 0000  r.J..... .......
00000010: 0001 0000 1000 0000 1000 0000 0800 0000  ................

typedef struct {
    uint32_t magic;         /* 72b5 4a86 */
    uint32_t version;       /* 0x0  */
    uint32_t headersize;    /* ox20 */
    uint32_t flags;         /* 0x01 */
    uint32_t numglyph;      /* 0x100 */
    uint32_t bytesperglyph; /* 0x10 */
    uint32_t height;        /* 0x10 */
    uint32_t width;         /* 0x08 */
} PSF_font;

$ readelf -sW font.o
Symbol table '.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_start
     2: 00000000000012a3     0 NOTYPE  GLOBAL DEFAULT    1 _binary_font_psf_end
     3: 00000000000012a3     0 NOTYPE  GLOBAL DEFAULT  ABS _binary_font_psf_size
```

## font.o内のunicodeテーブルを確認

```bash
_binary_font_psf_start : 0x40 (ELFヘッダーが先頭にあるため）
_binary_font_psf_end   : 0x12e3
*s = 0x40 + 0x020 + 0x100 * 0x10 = 0x1060
```

## uncodeテーブルの内容

```bash
00001060: efbf bdff ffff ffff ffff e280 a2ff ffff  ................
00001070: ffff ffff ffff ffff ffff c2b6 ffc2 a7ff  ................
00001080: ffff ffff ffff ffff ffff 20ff 21ff 22ff  .......... .!.".
00001090: 23ff 24ff 25ff 26ff 27ff 28ff 29ff 2aff  #.$.%.&.'.(.).*.
000010a0: 2bff 2cff 2dff 2eff 2fff 30ff 31ff 32ff  +.,.-.../.0.1.2.
000010b0: 33ff 34ff 35ff 36ff 37ff 38ff 39ff 3aff  3.4.5.6.7.8.9.:.
000010c0: 3bff 3cff 3dff 3eff 3fff 40ff 41ff 42ff  ;.<.=.>.?.@.A.B.
000010d0: 43ff 44ff 45ff 46ff 47ff 48ff 49ff 4aff  C.D.E.F.G.H.I.J.
000010e0: 4bff 4cff 4dff 4eff 4fff 50ff 51ff 52ff  K.L.M.N.O.P.Q.R.
000010f0: 53ff 54ff 55ff 56ff 57ff 58ff 59ff 5aff  S.T.U.V.W.X.Y.Z.
00001100: 5bff 5cff 5dff 5eff 5fff 60ff 61ff 62ff  [.\.].^._.`.a.b.
00001110: 63ff 64ff 65ff 66ff 67ff 68ff 69ff 6aff  c.d.e.f.g.h.i.j.
00001120: 6bff 6cff 6dff 6eff 6fff 70ff 71ff 72ff  k.l.m.n.o.p.q.r.
00001130: 73ff 74ff 75ff 76ff 77ff 78ff 79ff 7aff  s.t.u.v.w.x.y.z.
00001140: 7bff 7cff 7dff 7eff ffc3 87ff c3bc ffc3  {.|.}.~.........
00001150: a9ff c3a2 ffc3 a4ff c3a0 ffc3 a5ff c3a7  ................
00001160: ffc3 aaff c3ab ffc3 a8ff c3af ffc3 aeff  ................
00001170: c3ac ffc3 84ff c385 ffc3 89ff c3a6 ffc3  ................
00001180: 86ff c3b4 ffc3 b6ff c3b2 ffc3 bbff c3b9  ................
00001190: ffc3 bfff c396 ffc3 9cff c2a2 ffc2 a3ff  ................
000011a0: c2a5 ffff c692 ffc3 a1ff c3ad ffc3 b3ff  ................
000011b0: c3ba ffc3 b1ff c391 ffc2 aaff c2ba ffc2  ................
000011c0: bfff ffc2 acff c2bd ffc2 bcff c2a1 ffc2  ................
000011d0: abff c2bb ffe2 9691 ffe2 9692 ffe2 9693  ................
000011e0: ffe2 9482 ffe2 94a4 ffe2 95a1 ffe2 95a2  ................
000011f0: ffe2 9596 ffe2 9595 ffe2 95a3 ffe2 9591  ................
00001200: ffe2 9597 ffe2 959d ffe2 959c ffe2 959b  ................
00001210: ffe2 9490 ffe2 9494 ffe2 94b4 ffe2 94ac  ................
00001220: ffe2 949c ffe2 9480 ffe2 94bc ffe2 959e  ................
00001230: ffe2 959f ffe2 959a ffe2 9594 ffe2 95a9  ................
00001240: ffe2 95a6 ffe2 95a0 ffe2 9590 ffe2 95ac  ................
00001250: ffe2 95a7 ffe2 95a8 ffe2 95a4 ffe2 95a5  ................
00001260: ffe2 9599 ffe2 9598 ffe2 9592 ffe2 9593  ................
00001270: ffe2 95ab ffe2 95aa ffe2 9498 ffe2 948c  ................
00001280: ffe2 9688 ffff ffff ffce b1ff c39f ffce  ................
00001290: 93ff cf80 ffce a3ff cf83 ffc2 b5ff cf84  ................
000012a0: ffce a6ff ce98 ffce a9ff ceb4 ffe2 889e  ................
000012b0: ffcf 86ff ceb5 ffff ffc2 b1ff e289 a5ff  ................
000012c0: e289 a4ff ffff c3b7 ffe2 8988 ffc2 b0ff  ................
000012d0: e288 99ff c2b7 ffe2 889a ffff c2b2 ffff  ................
000012e0: c2a0 ff
```

# 抽出コード

- OSDevの[PC Screen Font](https://wiki.osdev.org/PC_Screen_Font)を使用

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

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

/* import the font contained in the object file created above */
extern char _binary_font_psf_start;
extern char _binary_font_psf_end;

uint16_t *unicode;

void psf_init()
{
    uint16_t glyph = 0;
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* is there a unicode table? */
    if (font->flags == 0) {
        unicode = NULL;
        return;
    }

    /* get the offset of the table */
    char *s = (char *)(
    (unsigned char*)&_binary_font_psf_start +
      font->headersize +
      font->numglyph * font->bytesperglyph
    );
    /* allocate memory for translation table */
    unicode = calloc(USHRT_MAX, 2);
    while(s < (unsigned char*)&_binary_font_psf_end) {
        uint16_t uc = (uint16_t)((unsigned char)s[0] & 0xff);   // この行を修正
        //printf("uc: 0x%x\n", uc);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
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

int main(void) {
    psf_init();
    for (int i = 0; i < 256; i++)
    	printf("unicode[%d] = %d\n", i, unicode[i]);

    return 0;
}
```

## 処理結果のunicode変換表

```bash
unicode[0] = 0
unicode[1] = 0
unicode[2] = 0
unicode[3] = 0
unicode[4] = 0
unicode[5] = 0
unicode[6] = 0
unicode[7] = 0
unicode[8] = 0
unicode[9] = 0
unicode[10] = 0
unicode[11] = 0
unicode[12] = 0
unicode[13] = 0
unicode[14] = 0
unicode[15] = 0
unicode[16] = 0
unicode[17] = 0
unicode[18] = 0
unicode[19] = 0
unicode[20] = 0
unicode[21] = 0
unicode[22] = 0
unicode[23] = 0
unicode[24] = 0
unicode[25] = 0
unicode[26] = 0
unicode[27] = 0
unicode[28] = 0
unicode[29] = 0
unicode[30] = 0
unicode[31] = 0
unicode[32] = 32
unicode[33] = 33
unicode[34] = 34
unicode[35] = 35
unicode[36] = 36
unicode[37] = 37
unicode[38] = 38
unicode[39] = 39
unicode[40] = 40
unicode[41] = 41
unicode[42] = 42
unicode[43] = 43
unicode[44] = 44
unicode[45] = 45
unicode[46] = 46
unicode[47] = 47
unicode[48] = 48
unicode[49] = 49
unicode[50] = 50
unicode[51] = 51
unicode[52] = 52
unicode[53] = 53
unicode[54] = 54
unicode[55] = 55
unicode[56] = 56
unicode[57] = 57
unicode[58] = 58
unicode[59] = 59
unicode[60] = 60
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
unicode[72] = 72
unicode[73] = 73
unicode[74] = 74
unicode[75] = 75
unicode[76] = 76
unicode[77] = 77
unicode[78] = 78
unicode[79] = 79
unicode[80] = 80
unicode[81] = 81
unicode[82] = 82
unicode[83] = 83
unicode[84] = 84
unicode[85] = 85
unicode[86] = 86
unicode[87] = 87
unicode[88] = 88
unicode[89] = 89
unicode[90] = 90
unicode[91] = 91
unicode[92] = 92
unicode[93] = 93
unicode[94] = 94
unicode[95] = 95
unicode[96] = 96
unicode[97] = 97
unicode[98] = 98
unicode[99] = 99
unicode[100] = 100
unicode[101] = 101
unicode[102] = 102
unicode[103] = 103
unicode[104] = 104
unicode[105] = 105
unicode[106] = 106
unicode[107] = 107
unicode[108] = 108
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
unicode[126] = 126
unicode[127] = 0
unicode[128] = 0
unicode[129] = 0
unicode[130] = 0
unicode[131] = 0
unicode[132] = 0
unicode[133] = 0
unicode[134] = 0
unicode[135] = 0
unicode[136] = 0
unicode[137] = 0
unicode[138] = 0
unicode[139] = 0
unicode[140] = 0
unicode[141] = 0
unicode[142] = 0
unicode[143] = 0
unicode[144] = 0
unicode[145] = 0
unicode[146] = 0
unicode[147] = 0
unicode[148] = 0
unicode[149] = 0
unicode[150] = 0
unicode[151] = 0
unicode[152] = 0
unicode[153] = 0
unicode[154] = 0
unicode[155] = 0
unicode[156] = 0
unicode[157] = 0
unicode[158] = 0
unicode[159] = 0
unicode[160] = 255
unicode[161] = 173
unicode[162] = 155
unicode[163] = 156
unicode[164] = 0
unicode[165] = 157
unicode[166] = 0
unicode[167] = 21
unicode[168] = 0
unicode[169] = 0
unicode[170] = 166
unicode[171] = 174
unicode[172] = 170
unicode[173] = 0
unicode[174] = 0
unicode[175] = 0
unicode[176] = 248
unicode[177] = 241
unicode[178] = 253
unicode[179] = 0
unicode[180] = 0
unicode[181] = 230
unicode[182] = 20
unicode[183] = 250
unicode[184] = 0
unicode[185] = 0
unicode[186] = 167
unicode[187] = 175
unicode[188] = 172
unicode[189] = 171
unicode[190] = 0
unicode[191] = 168
unicode[192] = 0
unicode[193] = 0
unicode[194] = 0
unicode[195] = 0
unicode[196] = 142
unicode[197] = 143
unicode[198] = 146
unicode[199] = 128
unicode[200] = 0
unicode[201] = 144
unicode[202] = 0
unicode[203] = 0
unicode[204] = 0
unicode[205] = 0
unicode[206] = 0
unicode[207] = 0
unicode[208] = 0
unicode[209] = 165
unicode[210] = 0
unicode[211] = 0
unicode[212] = 0
unicode[213] = 0
unicode[214] = 153
unicode[215] = 0
unicode[216] = 0
unicode[217] = 0
unicode[218] = 0
unicode[219] = 0
unicode[220] = 154
unicode[221] = 0
unicode[222] = 0
unicode[223] = 225
unicode[224] = 133
unicode[225] = 160
unicode[226] = 131
unicode[227] = 0
unicode[228] = 132
unicode[229] = 134
unicode[230] = 145
unicode[231] = 135
unicode[232] = 138
unicode[233] = 130
unicode[234] = 136
unicode[235] = 137
unicode[236] = 141
unicode[237] = 161
unicode[238] = 140
unicode[239] = 139
unicode[240] = 0
unicode[241] = 164
unicode[242] = 149
unicode[243] = 162
unicode[244] = 147
unicode[245] = 0
unicode[246] = 148
unicode[247] = 246
unicode[248] = 0
unicode[249] = 151
unicode[250] = 163
unicode[251] = 150
unicode[252] = 129
unicode[253] = 0
unicode[254] = 0
unicode[255] = 152
```
