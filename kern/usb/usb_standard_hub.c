//
// usbstandardhub.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <usb/usb_standard_hub.h>
#include <usb/usb_dev_factory.h>
#include <usb/usb.h>
#include <usb/usb_function.h>
#include <usb/usb_hub.h>
#include <usb/dwhc_device.h>
#include <usb/usb_request.h>
#include <usb/usb_dev_ns.h>
#include <types.h>
#include <console.h>
#include <mm.h>
#include <mm.h>
#include <string.h>

static uint32_t DEVNOS = 0;

static boolean usb_standard_hub_enumerate_ports(usb_standard_hub_t *self);

static uint32_t alloc_devno(void)
{
    uint32_t i;
    for (i = 0; i < 128; i++) {
        if ((DEVNOS & (1 << i)) == 0)
            break;
    }
    if (i < 128)
        DEVNOS |= (1 << i);
    // i == 64 はエラー
    return i;
}

void usb_standardhub(usb_standard_hub_t *self, usb_function_t *func)
{
    usb_function_copy(&self->func, func);
    self->func.configure = usb_standard_hub_config;
    self->intr_ep = 0;
    self->hub_desc = 0;
    self->buffer = 0;
    self->nports = 0;
    self->poweron = false;
    self->devno = 0;

    for (unsigned i = 0; i < USB_HUB_MAX_PORTS; i++) {
        self->devs[i] = 0;
        self->status[i] = 0;
        self->portconf[i] = false;
    }
}

void _usb_standardhub(usb_standard_hub_t *self)
{
    for (unsigned i = 0; i < self->nports; i++) {
        if (self->status[i] != 0) {
            //kmfree(self->status[i]);
            kfree(self->status[i]);
            self->status[i] = 0;
        }

        if (self->devs[i] != 0) {
            _usb_device(self->devs[i]);
            kmfree(self->devs[i]);
            self->devs[i] = 0;
        }
    }
    self->nports = 0;

    if (self->buffer != 0) {
        kmfree(self->buffer);
        self->buffer = 0;
    }

    if (self->intr_ep != 0) {
        kmfree(self->intr_ep);
        self->intr_ep = 0;
    }

    if (self->hub_desc != 0) {
        //kmfree(self->hub_desc);
        kfree(self->hub_desc);
        self->hub_desc = 0;
    }

    _usb_function(&self->func);
}

boolean usb_standard_hub_config(usb_function_t *func)
{
    // 1. usb_standardhubオブジェクトを取り出し
    usb_standard_hub_t *self = (usb_standard_hub_t *)func;
    assert (self != 0);

    // 2. EPの数は1であること
    if (usb_function_get_num_eps(&self->func) != 1) {
        warn("failed getting nums of endpoints");
        return false;
    }

    // 3. エンドポイントディスクリプタを取得
    const usb_ep_desc_t *ep_desc =
        (usb_ep_desc_t *) usb_function_get_desc(&self->func, DESCRIPTOR_ENDPOINT);
    // 入力EPかつ割り込みEPであること
    if (ep_desc == 0 || (ep_desc->addr & 0x80) != 0x80       // input EP
                     || (ep_desc->attr & 0x3F) != 0x03) {    // interrupt EP
        warn("bad endpoint descriptor");
        return false;
    }

    // 4. 割り込みエンドポイントの作成
    self->intr_ep = (usb_endpoint_t *)kmalloc(sizeof(usb_endpoint_t));
    usb_endpoint2(self->intr_ep, usb_function_get_dev(&self->func), ep_desc);

    // 5. ファンクションクラスとしてConfigure
    if (!usb_function_config(&self->func)) {
        warn("cannot set interface");
        return false;
    }

    // 6. ハブディスクリプタを取得して設定
    dwhc_device_t *host = usb_function_get_host(&self->func);
    // FIXME
    //self->hub_desc = (hub_desc_t *)kmalloc(sizeof(hub_desc_t));
    self->hub_desc = (hub_desc_t *)kalloc();
    assert(self->hub_desc != 0);
    if (dwhc_get_desc(host, usb_function_get_ep0(&self->func),
                    DESCRIPTOR_HUB, DESCRIPTOR_INDEX_DEFAULT,
                    self->hub_desc, sizeof(hub_desc_t),
                    REQUEST_IN | REQUEST_CLASS, 0) != (int) sizeof(hub_desc_t)) {
        warn("Cannot get hub descriptor");
        goto bad;
    }
    // 7. port数を設定
    self->nports = self->hub_desc->nports;
    if (self->nports > USB_HUB_MAX_PORTS) {
        warn("Too many ports (%d)", self->nports);
        goto bad;
    }

    // 8. デバイス番号を取得してデバイスを登録
    char *name = (char *)kmalloc(16);
    self->devno = alloc_devno();
    sprintf(name, "uhub%x", self->devno);
    usb_device_ns_add_dev(usb_device_ns_get(), name, self, false);
    kmfree(name);

    // 9. ポートのエニュメレーション
    if (!usb_standard_hub_enumerate_ports(self)) {
        warn("Port enumeration failed");
        goto bad;
    }

    // 10. プラグアンドプレイの場合は処理の変更リクエスを発行
/*
    dwhc_device_t *host = usb_function_get_host(&self->func);
    if (host->pap %% !usb_standard_hub_start_status_change_req(self)) {
        warn("Cannot start request");
        goto bad;
    }
*/

    return true;

bad:
    //kmfree(self->hub_desc);
    kfree(self->hub_desc);
    self->hub_desc = 0;
    return false;
}

static boolean
usb_standard_hub_enumerate_ports(usb_standard_hub_t *self)
{
    dwhc_device_t *host = usb_function_get_host(&self->func);
    usb_endpoint_t *ep0 = usb_function_get_ep0(&self->func);
    usb_speed_t speed;

    // 1. すべてのポートの電源をオン
    if (!self->poweron) {
        for (unsigned i = 0; i < self->nports; i++) {
            if (dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                SET_FEATURE, PORT_POWER, i+1, 0, 0) < 0) {
                warn("Cannot power port %d", i+1);
                return false;
            }
        }
        self->poweron = true;

        // elf->hub_desc->bPwrOn2PwrGood の値では低速デバイスでは十分でない
        // ようなので、ここでは最大値を使用する
        delayus(510*1000);
    }

    // 2. デバイスの検知、リセット、初期化を行う
    for (unsigned i = 0; i < self->nports; i++) {
        // 2.1 ポートにデバイスの登録があったら、再スキャンする
        if (self->devs[i] != 0) {
            usb_dev_rescan_dev(self->devs[i]);
            continue;
        }
        // 2.2 status[i]の領域を確保する
        if (self->status[i] == 0) {
            //self->status[i] = (usb_port_status_t *)kmalloc(sizeof(usb_port_status_t));
            self->status[i] = (usb_port_status_t *)kalloc();
            assert (self->status[i] != 0);
        }
        // 2.3 ポートのステータスを取得する
        if (dwhc_control_message(host, ep0,
            REQUEST_IN | REQUEST_CLASS | REQUEST_TO_OTHER,
            GET_STATUS, 0, i+1, self->status[i], 4) != 4) {
            warn("Cannot get status of port %d", i+1);
            continue;
        }
        // 2.4 電源が入っていること
        assert (self->status[i]->status & PORT_POWER__MASK);
        // 2.5 コネクトされていること
        if (!(self->status[i]->status & PORT_CONNECTION__MASK)) {
            continue;
        }
        // 2.6 ポートリセット
        if (dwhc_control_message(host, ep0,
            REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
            SET_FEATURE, PORT_RESET, i+1, 0, 0) < 0) {
            warn("Cannot reset port %d", i+1);
            continue;
        }
        delayus(100*1000);
        // 2.7 ポートステータスを取得する
        if (dwhc_control_message(host, ep0,
            REQUEST_IN | REQUEST_CLASS | REQUEST_TO_OTHER,
            GET_STATUS, 0, i+1, self->status[i], 4) != 4) {
            return false;
        }
        trace("Port %d status is 0x%04x", i+1, (unsigned) self->status[i]->status);

        // 2.8 ポートが利用可能になっていること
        if (!(self->status[i]->status & PORT_ENABLE__MASK)) {
            warn("Port %d is not enabled", i+1);
            continue;
        }

        // 2.9 過電流になっている場合は電源をオフしてエニュメレーションを中止
        if (self->status[i]->status & PORT_OVER_CURRENT__MASK) {
            dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, PORT_POWER, i+1, 0, 0);
            warn("Over-current condition on port %d", i+1);
            return false;
        }

        // 2.10 スピードの判定
        speed = usb_speed_unknown;
        if (self->status[i]->status & PORT_LOW_SPEED__MASK) {
            speed = usb_speed_low;
        } else if (self->status[i]->status & PORT_HIGH_SPEED__MASK) {
            speed = usb_speed_high;
        } else {
            speed = usb_speed_full;
        }
        // 2.11 ポートにデバイスを設定する
        assert (self->devs[i] == 0);
        self->devs[i] = kmalloc(sizeof(usb_dev_t));
        assert (self->devs[i] != 0);
        usb_device2(self->devs[i], host, speed, self, i);
        // 2.12 デバイスを初期化する
        if (!usb_dev_init(self->devs[i])) {
            _usb_device(self->devs[i]);
            kmfree(self->devs[i]);
            self->devs[i] = 0;
            continue;
        }
    }

    // 3. デバイスのコンフィグレーションを行う
    for (unsigned i = 0; i < self->nports; i++) {
        if (self->devs[i] == 0) continue;
        if (self->portconf[i]) continue;

        self->portconf[i] = true;
        if (!usb_dev_config(self->devs[i])) {
            warn("Port %d: Cannot configure device", i+1);
            _usb_device(self->devs[i]);
            kmfree(self->devs[i]);
            self->devs[i] = 0;
            continue;
        }

        info("Port %d: Device configured", i+1);
    }

    // 4. もう一度過電流がないかチェックする
    //usb_hub_status_t *hubstatus = kmalloc(sizeof(usb_hub_status_t));
    usb_hub_status_t *hubstatus = kalloc();
    assert (hubstatus != 0);
    // 4.1 ハブの過電流チェック
    if (dwhc_control_message(host, ep0,
            REQUEST_IN | REQUEST_CLASS,
            GET_STATUS, 0, 0, hubstatus, sizeof *hubstatus) != (int) sizeof *hubstatus) {
        warn("Cannot get hub status");
        //kmfree(hubstatus);
        kfree(hubstatus);
        return false;
    }

    if (hubstatus->status & HUB_OVER_CURRENT__MASK) {
        for (unsigned i = 0; i < self->nports; i++) {
            dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, PORT_POWER, i+1, 0, 0);
        }
        warn("Hub over-current condition");
        //kmfree (hubstatus);
        kfree (hubstatus);
        return false;
    }

    //kmfree(hubstatus);
    kfree(hubstatus);
    hubstatus = 0;

    boolean result = true;

    // 4.2 ポートの過電流チェック
    for (unsigned i = 0; i < self->nports; i++) {
        if (dwhc_control_message(host, ep0,
            REQUEST_IN | REQUEST_CLASS | REQUEST_TO_OTHER,
            GET_STATUS, 0, i+1, self->status[i], 4) != 4) {
            continue;
        }

        if (self->status[i]->status & PORT_OVER_CURRENT__MASK) {
            dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, PORT_POWER, i+1, 0, 0);
            warn("Over-current condition on port %d", i+1);
            result = false;
        }
    }

    return result;
}

boolean usb_standard_hub_rescan_dev(usb_standard_hub_t *self)
{
    return usb_standard_hub_enumerate_ports(self);
}

boolean usb_standard_hub_remove_dev(usb_standard_hub_t *self, uint32_t index)
{
    if (!usb_standard_hub_disable_port(self, index)) return false;

    kmfree(self->devs[index]);
    self->devs[index] = 0;

    return true;
}

boolean usb_standard_hub_disable_port(usb_standard_hub_t *self, uint32_t index)
{
    dwhc_device_t *host = usb_function_get_host(&self->func);
    usb_endpoint_t *ep0 = usb_function_get_ep0(&self->func);

    if (dwhc_control_message(host, ep0,
            REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
            CLEAR_FEATURE, PORT_ENABLE, index+1, 0, 0) < 0) {
        warn("Cannot disable port %d", index+1);
        return false;
    }

    self->portconf[index] = false;
    return true;
}

boolean usb_standard_hub_start_status_change_req(usb_standard_hub_t *self)
{
    size_t buflen = (self->nports + 8) / 8;
    dwhc_device_t *host = usb_function_get_host(&self->func);

    if (self->buffer == 0) {
        self->buffer = (uint8_t *)kmalloc(buflen);
    }
    // 割り込みEPにステータス変更リクエストを発行
    usb_request_t *urb = (usb_request_t *)kmalloc(sizeof(usb_request_t));
    usb_request(urb, self->intr_ep, self->buffer, buflen, 0);
    usb_request_set_comp_cb(urb, usb_standard_hub_comp_cbstub, 0, self);
    return dwhc_submit_async_request(host, urb, USB_TIMEOUT_NONE);
}

void usb_standard_hub_comp_cb(usb_standard_hub_t *self, usb_request_t *urb)
{
    if (urb->status != 0) {
        assert(urb->resultlen > 0);
        usb_standard_hub_port_status_changed(self);
    } else {
        if (urb->error == usb_err_frame_overrun)
            usb_standard_hub_start_status_change_req(self);
    }
    _usb_request(urb);
    kmfree(urb);
}

void usb_standard_hub_comp_cbstub(usb_request_t *urb, void *param, void *ctx)
{
    usb_standard_hub_t *self = (usb_standard_hub_t *)ctx;
    usb_standard_hub_comp_cb(self, urb);
}

void usb_standard_hub_handle_port_status_change(usb_standard_hub_t *self)
{
    dwhc_device_t *host = usb_function_get_host(&self->func);
    usb_endpoint_t *ep0 = usb_function_get_ep0(&self->func);

    uint16_t statbitmap = self->buffer[0];
    if (self->nports > 8) {
        assert(self->nports <= 15);
        statbitmap |= self->buffer[1] << 8;
    }
    if (statbitmap & 1)
        panic("Hub status change not handled");

    for (uint32_t i = 0; i < self->nports; i++) {
        if (!(statbitmap & (1 << (i + 1)))) continue;
        // ポートステータスを取得
        if (dwhc_control_message(host, ep0,
            REQUEST_IN | REQUEST_CLASS | REQUEST_TO_OTHER,
            GET_STATUS, 0, i+1, self->status[i], 4) != 4) {
            panic("Cannot get port status (port %d)", i+1);
        }

        uint16_t change = self->status[i]->change;
        // サスペンドと過電流の変更は未サポート
        assert(!(change & C_PORT_SUSPEND__MASK));      // TODO
        assert(!(change & C_PORT_OVER_CURRENT__MASK)); // TODO
        // PORT_ENABLEが変更; ポートの無効に
        if (change & C_PORT_ENABLE__MASK) {
            if (dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, C_PORT_ENABLE, i+1, 0, 0) < 0) {
                panic("Cannot clear C_PORT_ENABLE (port %d)", i+1);
            }
        }
        // PORT_RESETが変更: リセット機能をクリア
        if (change & C_PORT_RESET__MASK) {
            if (dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, C_PORT_RESET, i+1, 0, 0) < 0) {
                panic("Cannot clear C_PORT_RESET (port %d)", i+1);
            }
        }
        // PORT_CONNECTIONが変更
        if (change & C_PORT_CONNECTION__MASK) {
            if (dwhc_control_message(host, ep0,
                REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_OTHER,
                CLEAR_FEATURE, C_PORT_CONNECTION, i+1, 0, 0) < 0) {
                panic("Cannot clear C_PORT_CONNECTION (port %d)", i+1);
            }

            if (self->status[i]->status & PORT_CONNECTION__MASK) {
                if (self->devs[i] == 0) {
                    usb_standard_hub_rescan_dev(self);
                }
            } else {
                if (self->devs[i] != 0) {
                    usb_standard_hub_remove_dev(self, i);
                }
            }
        }
    }
    // ステータス変更リクエストを再開
    if (!usb_standard_hub_start_status_change_req(self)) {
        warn("Cannot restart request");
    }
}

void usb_standard_hub_port_status_changed(usb_standard_hub_t *self)
{
/*
    dwhc_device_t *host = usb_function_get_host(&self->func);
    if (dwhc_is_pap(host))
    {
        port_status_event_t *event = (port_status_event_t*)kmalloc(sizeof(port_status_event_t));
        assert (event != 0);
        event->from_root_port = false;
        event->hub    = self;
        list_init(&event->list);

        acquire(&host->hublock);
        list_push_back(&host->hublist, &event->list);
        release(&host->hublock);
    }
*/
}
