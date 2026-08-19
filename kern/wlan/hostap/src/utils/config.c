/*
 * Configuration parsing
 * Copyright (c) 2003-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <utils/includes.h>

#include <utils/config.h>
#include <utils/common.h>
#include <fs/fatfs/ff.h>


static int newline_terminated(const char *buf, size_t buflen)
{
	size_t len = os_strlen(buf);
	if (len == 0)
		return 0;
	if (len == buflen - 1 && buf[buflen - 1] != '\r' &&
	    buf[len - 1] != '\n')
		return 0;
	return 1;
}


static void skip_line_end(FILE *stream)
{
	char buf[100];
	while (fgets(buf, sizeof(buf), stream)) {
		buf[sizeof(buf) - 1] = '\0';
		if (newline_terminated(buf, sizeof(buf)))
			return;
	}
}

// ファイルから1行取り出し、行頭行末のスペース、#で始まるコメントを削除し、
// 行頭のポインタを_posにセットし、そのポインタを返す
char * wpa_config_get_line(char *s, int size, FILE *stream, int *line,
			   char **_pos)
{
	char *pos, *end, *sstart;

	while (fgets(s, size, stream)) {
		(*line)++;
		s[size - 1] = '\0';
		if (!newline_terminated(s, size)) {
			/*
			 * The line was truncated - skip rest of it to avoid
			 * confusing error messages.
			 */
			wpa_printf(MSG_INFO, "Long line in configuration file "
				   "truncated");
			skip_line_end(stream);
		}
		pos = s;

		/* Skip white space from the beginning of line. */
		while (*pos == ' ' || *pos == '\t' || *pos == '\r')
			pos++;

		/* Skip comment lines and empty lines */
		if (*pos == '#' || *pos == '\n' || *pos == '\0')
			continue;

		/*
		 * 二重引用符で囲まれた文字列内にある場合を除き、#で始まるコメントを削除する。
		 */
		sstart = pos;
		end = os_strchr(sstart, '#');
		while (end) {
			sstart = os_strchr(sstart, '"');
			if (!sstart || sstart > end)
				break;
			sstart = os_strchr(sstart + 1, '"');
			if (!sstart)
				break;
			sstart++;
			if (sstart > end)
				end = os_strchr(sstart, '#');
		}

		if (end)
			*end-- = '\0';
		else
			end = pos + os_strlen(pos) - 1;

		/* 行末の空白類を削除する. */
		while (end > pos &&
		       (*end == '\n' || *end == ' ' || *end == '\t' ||
			*end == '\r'))
			*end-- = '\0';

		if (*pos == '\0')
			continue;

		if (_pos)
			*_pos = pos;
		return pos;
	}

	if (_pos)
		*_pos = NULL;
	return NULL;
}
