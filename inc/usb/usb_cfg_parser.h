#ifndef INC_USb_usb_cfg_parser_H
#define INC_USb_usb_cfg_parser_H

#include <usb/usb.h>
#include <types.h>

typedef struct usb_usb_cfg_parser {
    const usb_desc_t *buffer;   ///< ディスクリプタの入ったバッファ
    unsigned    buflen;         ///< ディスクリプタの合計サイズ
    boolean     valid;          ///< ディスクリプタデータとして正しいか
    const usb_desc_t *end_pos;  ///< 最後のディスクリプタのポインタ
    const usb_desc_t *next_pos; ///< 次のディスクリプタのポインタ
    const usb_desc_t *curr_desc; ///< 現在のディスクリプタのポインタ
    const usb_desc_t *err_pos;  ///< エラーが発生したディスクリプタのポインタ
} usb_usb_cfg_parser_t;

void usb_cfg_parser(usb_usb_cfg_parser_t *self, void *buffer, unsigned buflen);
void usb_cfg_parser_copy(usb_usb_cfg_parser_t *self, usb_usb_cfg_parser_t *parser);
void _usb_cfg_paser(usb_usb_cfg_parser_t *self);

boolean usb_cfg_parser_is_valid(usb_usb_cfg_parser_t *self);

const usb_desc_t *usb_cfg_parser_get_desc(usb_usb_cfg_parser_t *self, uint8_t type);

const usb_desc_t *usb_cfg_parser_get_curr_desc(usb_usb_cfg_parser_t *self);

void usb_cfg_parser_error(usb_usb_cfg_parser_t *self, const char *source);

#endif
