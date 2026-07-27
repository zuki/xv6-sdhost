/*
 * Configuration parsing
 * Copyright (c) 2003-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_UTILS_CONFIG_H
#define INC_WLAN_UTILS_CONFIG_H

#include <fs/fatfs/ff.h>

#define FILE	FIL

#define fclose	f_close
#define fgets	f_gets


/**
 * wpa_config_get_line - 構成ファイルの次の行を読み込む
 * @s: 行を読み込むバッファ
 * @size: バッファ長
 * @stream: データを読み込むファイルストリーム
 * @line: ファイルの行番号を格納している変数へのポインタ
 * @_pos: テキスト行上のデータの先頭へのポインタ用のバッファ、
 * 必要がない（使用する値を返す）場合は %NULL
 * Returns: テキスト行のデータの先頭へのポインタ。テキスト行が
 * これ以上ない場合は %NULL
 *
 * この関数は構成ファイルから次の空でない行を読み込み、コメントを削除する。
 * 返される文字列はヌル終端されていることが保証されている。
 */
char * wpa_config_get_line(char *s, int size, FILE *stream, int *line,
			   char **_pos);

#endif /* UTILS_CONFIG_H */
