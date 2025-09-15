//
// From dwhcidevice.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright(C) 2014-2022  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// DesignWave USB2 Host Controller device
#ifndef H_USB_DWHC_DEVICE_H
#define H_USB_DWHC_DEVICE_H

#include <usb/usb.h>
#include <usb/usb_endpoint.h>
#include <usb/usb_request.h>
#include <usb/dwhc_root_port.h>
#include <usb/dwhc_xfer_data.h>
#include <usb/dwhc_regs.h>
#include <usb/usb_standard_hub.h>
#include <list.h>
#include <types.h>
#include <spinlock.h>
#include <usb/synchronize.h>

#define DWHCI_WAIT_BLOCKS   DWHCI_MAX_CHANNELS

/// @brief DesignWave USB2 ホストコントローラ構造体
typedef struct dwhc_device {
    unsigned            channels;               ///< チャネル数
    volatile unsigned   allocated_chans;        ///< 割り当て済みチャネル数: 1ビット1チャネル
    struct spinlock     chanlock;               ///< スピンロック: allocated_chansを保護
    struct spinlock     wblklock;               ///< スピンロック: allocated_wkblockを保護
    struct spinlock     imasklock;              ///< スピンロック: チャネル割り込みのマスク/アンマスク処理を保護
    dwhc_xfer_data_t *stdata[DWHCI_MAX_CHANNELS]; ///< 転送ステートデータ（配列は各チャネルに対応）
    volatile boolean    waiting[DWHCI_WAIT_BLOCKS];  ///< 待機中フラグ
    volatile unsigned   allocated_wkblock;      ///< 待機中フラグ配列の割り当て済みウェイトブロック: 1ビット1wait block
    dwhc_root_port_t    root_port;              ///< ルートポート
    volatile boolean    root_port_enabled;      ///< ルートポートが有効か
    volatile boolean    shutdown;               ///< USBドライバはshutdownするか

#ifdef USE_USB_SOF_INTR
    struct list_head    tqueue;                 ///< トランザクションキュー
#endif
// 以下、usbhostcontrollerから
    struct spinlock     hublock;                ///< スピンロック: ハブリストを保護
    struct list_head    hublist;                ///< ハブリスト
    /* boolean             pap; */              ///< プラグアンドプレイは有効か
} dwhc_device_t;

/// @brief ポートステータスイベント構造体
typedef struct port_status_event {
    boolean              from_root_port;  // ルートハブからか
    union {
        dwhc_root_port_t    *root_port;
        usb_standard_hub_t  *hub;
    };
    struct list_head     list;
} port_status_event_t;

// PUBLIC
void dwhc_device(dwhc_device_t *self);
void _dwhc_device(dwhc_device_t *self);

/// @brief DWHCデバイスの初期化
/// @param scan 初期化の最後にデバイスを再スキャンするか
/// @return 成功したらtrue, 失敗したらfalse
boolean dwhc_init(dwhc_device_t *self, boolean scan);

/// @brief ルートポートが初期化されていなかったら初期化してUSBデバイスが接続されていないかスキャン
void dwhc_rescan_dev(dwhc_device_t *self);

/// @brief リクエストをブロッキングモードで発行する（アイソクロナス転送は未サポート）
/// @param self USBホストコントローラ
/// @param urb リクエスト
/// @param timeout タイムアウト時間（デフォルトは0: なし）
/// @return true, 失敗したらfalse
boolean dwhc_submit_block_request(dwhc_device_t *self, usb_request_t *urb, unsigned timeout);

/// @brief リクエストの非同期転送
/// @param urb USBリクエストオブジェクト
/// @param timeout タイムアウト時間（デフォルトは0: なし）
/// @return 操作の成否
boolean dwhc_submit_async_request(dwhc_device_t *self, usb_request_t *urb, unsigned timeout);

/// @brief 指定デバイスのトランザクションをキャンセルする
/// @param device キャンセルするデバイス
void dwhc_cancel_device_transaction(dwhc_device_t *self, usb_dev_t *dev);

// static関数

/// @brief ホストポートはコネクトされているか
/// @return されていればtrue, されていなければfalse
boolean dwhc_device_connected(dwhc_device_t *self);

// @brief ホストポートのスピードを取得する
/// @return ホストスピード
usb_speed_t dwhc_get_port_speed(dwhc_device_t *self);

/// @brief 過電流がないか
/// @return 過電流があればTRUE
boolean dwhc_overcurrent_detected(dwhc_device_t *self);

/// @brief ホストのルートポートを無効化して、電源を切る（bPowerOff = TRUEの場合）
/// @param poweroff 電源を切るか
void dwhc_disable_root_port(dwhc_device_t *self, boolean poweroff);

/// @brief コアの初期化
/// @return 操作の成否
boolean dwhc_init_core(dwhc_device_t *self);

/// @brief dwhcホストの初期化
/// @return 常にTRUE
boolean dwhc_init_host(dwhc_device_t *self);

/// @brief ルートポートの有効化
/// @return 操作の成否
boolean dwhc_enable_root_port(dwhc_device_t *self);

/// @brief dwhcホストの電源オン
/// @return 操作の成否
boolean dwhc_power_on(dwhc_device_t *self, uint32_t poweron);

/// @brief dwhcホストのリセット
/// @return 操作の成否
boolean dwhc_reset(dwhc_device_t *self);

/// @brief グローバル割り込みの有効化
void dwhc_enable_global_intr(dwhc_device_t *self);

/// @brief 一般割り込みの有効化
void dwhc_enable_common_intr(dwhc_device_t *self);

/// @brief dwhcホスト割り込みの有効化
void dwhc_enable_host_intr(dwhc_device_t *self);

/// @brief チャネルの割り込みの有効化
/// @param channel 有効にするチャネル番号
void dwhc_enable_channel_intr(dwhc_device_t *self, unsigned channel);

/// @brief チャネルの割り込みの無効化
/// @param channel 無効にするチャネル番号
void dwhc_disable_channel_intr(dwhc_device_t *self, unsigned channel);

/// @brief 送信FIFOのフラッシュ
/// @param nFIFO FIFO番号
void dwhc_flush_tx_fifo(dwhc_device_t *self, unsigned fifo);

/// @brief 受信FIFOのフラッシュ
void dwhc_flush_rx_fifo(dwhc_device_t *self);

/// @brief リクエストの転送処理.\n
///     実際の転送自体はdwhc_xfer_stage_async()で行う
/// @param urb USBリクエストオブジェクト
/// @param in INリクエストか
/// @param stage  ステータスステージか
/// @param timeout タイムアウト
/// @return
boolean dwhc_xfer_stage(dwhc_device_t *self, usb_request_t *urb, boolean in, boolean stage, unsigned timeout);

/// @brief 転送完了時のコールバック関数
/// @param urb リクエスト
/// @param param リクエストパラメタ（waiting配列のインデックスを設定）
/// @param ctx コンテキスト（USBホストコントローラオブジェクトを設定）
void dwhc_comp_cb(usb_request_t *urb, void *param, void *ctx);

/// @brief リクエストの非同期転送.\n
///     実際の転送を非同期で行う
/// @param urb USBリクエストオブジェクト
/// @param in INリクエストか
/// @param stage ステータスステージか
/// @param timeout タイムアウト
/// @return
boolean dwhc_xfer_stage_async(dwhc_device_t *self, usb_request_t *urb, boolean in, boolean stage, unsigned timeout);

#ifdef USE_USB_SOF_INTR
/// @brief トランザクションをキューに入れる
/// @param data トランザクションステージデータ
void dwhc_queue_trans(dwhc_device_t *self, dwhc_xfer_data_t *data);
/// @brief 遅延を入れてトランザクションをキューに入れる
/// @param data トランザクションステージデータ
void dwhc_queue_delay_trans(dwhc_device_t *self, dwhc_xfer_data_t *data);
#endif

/// @brief トランザクションを開始する
/// @param data トランザクションステージデータ
void dwhc_start_trans(dwhc_device_t *self, dwhc_xfer_data_t *stdata);

/// @brief チャネルを開始する
/// @param data トランザクションステージデータ
void dwhc_start_channel(dwhc_device_t *self, dwhc_xfer_data_t *stdata);

/// @brief チャネル割り込みハンドラ
/// @param channel チャネル番号
void dwhc_channel_intr_hdl(dwhc_device_t *self, unsigned channel);
#ifdef USE_USB_SOF_INTR
/// @brief SOF割り込みハンドラ
void dwhc_sof_intr_hdl(dwhc_device_t *self);
#endif
/// @brief 割り込みハンドラ
void dwhc_intr_hdl(dwhc_device_t *self);
/// @brief 割り込みスタブ
/// @param param パラメタ（dwhciデバイスへのポインタ）
void dwhc_intr_hdlstub(void *param);

#ifndef USE_USB_SOF_INTR
    void dwhc_device_timer_hdl(dwhc_device_t *self, dwhc_xfer_data_t *stdata);
    void dwhc_device_timer_hdlstub(uint64_t data);
#endif

/// @brief 転送用のチャネルを割り当てる
/// @return チャネル番号
unsigned dwhc_alloc_channel(dwhc_device_t *self);

/// @brief チャネルを開放する
/// @param channel チャネル番号
void dwhc_free_channel(dwhc_device_t *self, unsigned channel);

/// @brief waitblockを割り当てる. \n
///    waitblockは配列でDWHCI_WAIT_BLOCKS個用意されている。そのうち、未使用の最初のwaitblockをシユすることにして、そのインデックスを返す
/// @return waitblockのインデックス
unsigned dwhc_alloc_wblock(dwhc_device_t *self);

/// @brief waitblockを開放する
/// @param wblock 待機ブロック
void dwhc_free_wblock(dwhc_device_t *self, unsigned wblock);

/// @brief 指定のビットが立つのを待つ
/// @param register 対象レジスタ
/// @param mask 対象ビット
/// @param bit 待機するビット値（trueかfalseか）
/// @param timeout タイムアウト
/// @return ビットが立ったらTRUE, タイムアウトになったらFALSE
boolean dwhc_wait_for_bit(dwhc_device_t *self, uint64_t reg, uint32_t mask, boolean bit, unsigned timeout);

/// @brief レジスタ値をダンプ
/// @param name レジスタ名
/// @param addr レジスタアドレス
void dwhc_dumpreg(dwhc_device_t *self,const char *name, uint32_t addr);
/// @brief ステータスをダンプ
/// @param channel チャネル番号
void dwhc_dump_status (dwhc_device_t *self, unsigned channel);

// 以下は、usbhostcontrollerから

/// @brief ディスクリプタを取得する
/// @param ep エンドポイント
/// @param type ディスクリプタのタイプ
/// @param index インデックス
/// @param buffer バッファ
/// @param buflen バッファのサイズ
/// @param reqtype リクエストタイプ
/// @param wIndex エンドポイント(0)、インタフェース、言語IDのいずれか
/// @return 長さ、失敗した場合は負値
int dwhc_get_desc(dwhc_device_t *self, usb_endpoint_t *ep,
                  unsigned char type, unsigned char index,
                  void *buffer, unsigned buflen,
                  unsigned char reqtype, unsigned short idx);

/// @brief デバイスにアドレスを付与する
/// @param ep エンドポイント
/// @param addr デバイスに付与するアドレス
/// @return 成功したらTRUE, 失敗したらFALSE
boolean dwhc_set_addr(dwhc_device_t *self, usb_endpoint_t *ep, uint8_t addr);

///@brief デバイスにコンフィグレーションをセットする
/// @param ep エンドポイント
/// @param cfgvalue コンフィグレーション値
/// @return 成功したらTRUE, 失敗したらFALSE
boolean dwhc_set_config(dwhc_device_t *self, usb_endpoint_t *ep, uint8_t cfgvalue);

/// @brief コントロール転送でメッセージを送信する
/// @param ep エンドポイント
/// @param reqtype リクエストタイプ
/// @param req リクエストコマンド
/// @param value 値
/// @param index インデックス
/// @param data データ
/// @param datalen データサイズ
/// @return 長さ、失敗した場合は負値
int dwhc_control_message(dwhc_device_t *self, usb_endpoint_t *ep,
            uint8_t reqtype, uint8_t req, uint16_t value, uint16_t index,
            void *data, uint16_t datalen);

/// @brief 転送を行う
/// @return 実際に送信した長さ、失敗の場合は負値
int dwhc_xfer(dwhc_device_t *self, usb_endpoint_t *ep, const void *buffer, unsigned buflen);

/* plug and play関連
static inline boolean dwhc_is_pap(dwhc_device_t *self) {
    return self->pap;
}

/// @brief デバイスツリーが更新されたか（TASK_LEVELで呼び出す必要がある）
/// @return プラグアンドプレイが有効な場合、デバイスツリーが更新された場合にtrue返す。\n
/// （最初に呼び出されたときは常にtrueを返す）
boolean dwhc_update_pap(dwhc_device_t *self);
*/

/// @brief レジスタname(アドレスaddr)をダンプ出力する
/// @param self USBホストコントローラ
/// @param name レジスタ名
/// @param addr レジスタアドレス
void dwhc_dumreg(dwhc_device_t *self, const char *name, uint64_t addr);

/// @brief チャネルchannel関連のすべてのレジスタをダンプ出力する
/// @param self USBホストコントローラ
/// @param channel チャネル
void dwhc_dump_status(dwhc_device_t *self, unsigned channel);

/// @brief USBホストコントローラを初期化する
void usbhc_init(void);

/// @brief usbホストをテストする
void usb_test(void);

#endif
