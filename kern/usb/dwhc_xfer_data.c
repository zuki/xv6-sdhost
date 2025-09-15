//
// dwhcixferstagedata.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright(C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include <usb/dwhc_xfer_data.h>
#include <usb/dwhc_periodic.h>
#include <usb/dwhc_non_periodic.h>
#include <usb/dwhc_non_split.h>
#include <usb/dwhc_regs.h>
#include <usb/usb_request.h>
#include <types.h>
#include <console.h>
#include <debug.h>
#include <linux/time.h>

#define MAX_BULK_TRIES      8

#define BTF(x) ((x) ? "true" : "false")

void
debug_stdata(dwhc_xfer_data_t *self)
{
    cprintf("\nstagedata: stagedata: 0x%p\n", self);
    cprintf("    : channel: %d, in: %s, stage: %s, fsused: %s, split: %s\n",
        self->channel, BTF(self->in), BTF(self->ststatus), BTF(self->fsused), BTF(self->split));
    cprintf("    : state: %d, substate: %d, trstatus: %d\n",
        self->state, self->substate, self->trstatus);
    cprintf("    : buffp: 0x%p, bpt: %d, ppt: %d, packets: %d\n\n",
        self->buffp, self->bpt, self->ppt, self->packets);
}

void dwhc_xfer_data(dwhc_xfer_data_t *self, unsigned channel, usb_request_t *urb, boolean in, boolean ststatus, unsigned timeout)
{
    assert(self != 0);

    self->channel = channel;
    self->urb = urb;
    self->in = in;
    self->timeout = USB_TIMEOUT_NONE;
    self->ststatus = ststatus;
    self->split_comp = false;
    self->xfered = 0;
    self->state = 0;
    self->substate = 0;
    self->trstatus = 0;
    self->fsused = false;
    self->err_cnt = 0;
    self->start = 0;

    self->ep = usb_request_get_ep(urb);
    self->dev = usb_endpoint_get_device(self->ep);
    self->speed = usb_dev_get_speed(self->dev);
    self->xpsize = usb_endpoint_get_max_packet_size(self->ep);

    self->split = self->dev->hubaddr != 0 && self->speed != usb_speed_high;

    if (!ststatus) {
        if (usb_endpoint_get_nextpid(self->ep, ststatus) == usb_pid_setup) {
            self->buffp = urb->setup_data;
            self->xfersize = sizeof(usb_setup_data_t);
        } else {
            self->buffp = urb->buffer;
            self->xfersize = usb_request_get_buflen(urb);
        }

        self->packets =(self->xfersize + self->xpsize - 1) / self->xpsize;

        if (self->split) {
            if (self->xfersize > self->xpsize) {
                self->bpt = self->xpsize;
            } else {
                self->bpt = self->xfersize;
            }

            self->ppt = 1;
        } else {
            self->bpt = self->xfersize;
            self->ppt = self->packets;
        }
    } else {
        self->buffp = &self->buffer;

        self->xfersize = 0;
        self->bpt = 0;
        self->packets = 1;
        self->ppt = 1;
    }

    assert(self->buffp != 0);
    //trace("bp=0x%llx", self->buffp);
    //if (self->buffp == 0 || ((uintptr_t) self->buffp & 3) != 0) {
    //    error("buffp not align: %p", self->buffp);
    //}
    assert(((uintptr_t) self->buffp & 3) == 0);

    if (self->split) {
        if (dwhc_xfer_data_is_periodic(self)) {
            dwhc_periodic(&self->scheduler.periodic);
            debug("periodic: 0x%p", &self->scheduler.periodic);
        } else {
            dwhc_non_periodic(&self->scheduler.nonperiodic);
            debug("nonperiodic: 0x%p", &self->scheduler.nonperiodic);
        }
        self->fsused = true;
    } else {
        if (usb_dev_get_hubaddr(self->dev) == 0 && self->speed != usb_speed_high)
        {
            dwhc_non_split(&self->scheduler.nosplit,
                            dwhc_xfer_data_is_periodic(self));
            debug("nosplit: 0x%p", &self->scheduler.nosplit);
            self->fsused = true;
        } else {
            debug("no use scheduler");
        }
    }

    if (timeout != USB_TIMEOUT_NONE) {
        assert(self->ep->type == ep_type_interrupt);

        self->timeout = timeout * HZ / 1000;
        self->start = jiffies;
    }
    debug_stdata(self);
}

void _dwhc_xfer_data(dwhc_xfer_data_t *self)
{
    if (self->fsused) {
        self->scheduler.base._scheduler(&self->scheduler.base);
    }

    self->buffp = 0;

    self->ep = 0;
    self->dev = 0;
    self->urb = 0;
}

void dwhc_xfer_data_trans_complete(dwhc_xfer_data_t *self, uint32_t status, uint32_t packetleft, uint32_t byteleft)
{
    self->trstatus = status;

    if (status & (DWHCI_HOST_CHAN_INT_ERROR_MASK
                | DWHCI_HOST_CHAN_INT_NAK
                | DWHCI_HOST_CHAN_INT_NYET)) {
        if (status & DWHCI_HOST_CHAN_INT_NAK && self->urb->onnak) {
            assert(self->in);   // IN転送であること
            self->packets = 0;  // 利用可能なデータなし、転送を完了
            return;
        }

        // xactエラーが生じたバルク転送は再試行する。それ以外はreturn
        if (!(status & DWHCI_HOST_CHAN_INT_XACT_ERROR)
           || self->ep->type != ep_type_bulk
           || ++self->err_cnt > MAX_BULK_TRIES) {
            return;
         }
    }

    uint32_t packetxfered = self->ppt - packetleft;
    uint32_t bytexfered   = self->bpt - byteleft;

    if (self->split && self->split_comp && bytexfered == 0 && self->bpt > 0) {
        bytexfered = self->xpsize * packetxfered;
    }

    self->xfered += bytexfered;
    self->buffp = (uint8_t *)self->buffp + bytexfered;

    if (!self->split || self->split_comp) {
        usb_endpoint_skip_pid(self->ep, packetxfered, self->ststatus);
    }

    // これはないはずだが、何らかのデバイスで生じるようだ
    if (packetxfered > self->packets) {
        self->trstatus |= DWHCI_HOST_CHAN_INT_FRAME_OVERRUN;
        self->err_cnt = MAX_BULK_TRIES + 1;
        self->packets = 0;
        return;
    }

    self->packets -= packetxfered;

    if (!self->split) {
        self->ppt = self->packets;
    }

    // (xfersize > xfersize) の場合、これはfalseになる
    if (self->xfersize - self->xfered < self->bpt) {
        assert(self->xfered <= self->xfersize);
        self->bpt = self->xfersize - self->xfered;
    }
}

void dwhc_xfer_data_set_split_complete(dwhc_xfer_data_t *self, boolean comp)
{
    self->split_comp = comp;
}

void dwhc_xfer_data_set_state(dwhc_xfer_data_t *self, unsigned state)
{
    self->state = state;
}

unsigned dwhc_xfer_data_get_state(dwhc_xfer_data_t *self)
{
    return self->state;
}

void dwhc_xfer_data_set_sub_state(dwhc_xfer_data_t *self, unsigned substate)
{
    self->substate = substate;
}

unsigned dwhc_xfer_data_get_sub_state(dwhc_xfer_data_t *self)
{
    return self->substate;
}

/// @brief スプリットサイクルを開始する
/// @param self ステージデータ構造体へのポインタ
/// @return 開始したか
boolean dwhc_xfer_data_begin_split_cycle(dwhc_xfer_data_t *self)
{
    return true;
}

unsigned dwhc_xfer_data_get_channel_number(dwhc_xfer_data_t *self)
{
    return self->channel;
}

boolean dwhc_xfer_data_is_periodic(dwhc_xfer_data_t *self)
{
    usb_endpoint_type_t type = usb_endpoint_get_type(self->ep);

    return type == ep_type_interrupt || type == ep_type_isochronous;
}

uint8_t dwhc_xfer_data_get_dev_addr(dwhc_xfer_data_t *self)
{
    return usb_dev_get_addr(self->dev);
}

uint8_t dwhc_xfer_data_get_ep_type(dwhc_xfer_data_t *self)
{
    unsigned type = 0;

    switch(usb_endpoint_get_type(self->ep))
    {
    case ep_type_control:
        type = DWHCI_HOST_CHAN_CHARACTER_EP_TYPE_CONTROL;
        break;

    case ep_type_bulk:
        type = DWHCI_HOST_CHAN_CHARACTER_EP_TYPE_BULK;
        break;

    case ep_type_interrupt:
        type = DWHCI_HOST_CHAN_CHARACTER_EP_TYPE_INTERRUPT;
        break;

    default:
        warn("bad ep_type: %d", usb_endpoint_get_type(self->ep));
        //assert(0);
        type = DWHCI_HOST_CHAN_CHARACTER_EP_TYPE_CONTROL;
        break;
    }

    return type;
}

uint8_t dwhc_xfer_data_get_ep_number(dwhc_xfer_data_t *self)
{
    return usb_endpoint_get_number(self->ep);
}

uint32_t dwhc_xfer_data_get_max_packet_size(dwhc_xfer_data_t *self)
{
    return self->xpsize;
}

usb_speed_t dwhc_xfer_data_get_speed(dwhc_xfer_data_t *self)
{
    return self->speed;
}

uint8_t dwhc_xfer_data_get_pid(dwhc_xfer_data_t *self)
{
    uint8_t pid = 0;
    usb_pid_t next = usb_endpoint_get_nextpid(self->ep, self->ststatus);
    switch(next)
    {
    case usb_pid_setup:
        pid = DWHCI_HOST_CHAN_XFER_SIZ_PID_SETUP;
        break;

    case usb_pid_data0:
        pid = DWHCI_HOST_CHAN_XFER_SIZ_PID_DATA0;
        break;

    case usb_pid_data1:
        pid = DWHCI_HOST_CHAN_XFER_SIZ_PID_DATA1;
        break;

    default:
        warn("bad next: %d", next);
        assert(0);
        break;
    }

    return pid;
}

boolean dwhc_xfer_data_is_in(dwhc_xfer_data_t *self)
{
    return self->in;
}

boolean dwhc_xfer_data_is_ststage(dwhc_xfer_data_t *self)
{
    return self->ststatus;
}

uint64_t dwhc_xfer_data_get_dmaaddr(dwhc_xfer_data_t *self)
{
    return (uint64_t)self->buffp;
}

uint32_t dwhc_xfer_data_get_bpt(dwhc_xfer_data_t *self)
{
    return self->bpt;
}

uint32_t dwhc_xfer_data_get_ppt(dwhc_xfer_data_t *self)
{
    return self->ppt;
}

boolean dwhc_xfer_data_is_split(dwhc_xfer_data_t *self)
{
    return self->split;
}

boolean dwhc_xfer_data_is_split_complete(dwhc_xfer_data_t *self)
{
    return self->split_comp;
}

uint8_t dwhc_xfer_data_get_hubaddr(dwhc_xfer_data_t *self)
{
    return usb_dev_get_hubaddr(self->dev);
}

uint8_t dwhc_xfer_data_get_hubport(dwhc_xfer_data_t *self)
{
     return usb_dev_get_hubport(self->dev);
}

uint8_t dwhc_xfer_data_get_split_pos(dwhc_xfer_data_t *self)
{
    // only important for isochronous transfers
    return DWHCI_HOST_CHAN_SPLIT_CTRL_ALL;
}

uint32_t dwhc_xfer_data_get_status_mask(dwhc_xfer_data_t *self)
{
    uint32_t mask =   DWHCI_HOST_CHAN_INT_XFER_COMPLETE
            | DWHCI_HOST_CHAN_INT_HALTED
            | DWHCI_HOST_CHAN_INT_ERROR_MASK;

    if (self->split || dwhc_xfer_data_is_periodic(self)) {
        mask |=   DWHCI_HOST_CHAN_INT_ACK
             | DWHCI_HOST_CHAN_INT_NAK
             | DWHCI_HOST_CHAN_INT_NYET;
    }
#ifdef USE_NAK_USB_FIX
    else if (usb_request_is_comp_on_nak(self->urb)) {
        mask |= DWHCI_HOST_CHAN_INT_NAK;
    }
#endif

    return mask;
}

uint32_t dwhc_xfer_data_get_trstatus(dwhc_xfer_data_t *self)
{
    return self->trstatus;
}

boolean dwhc_xfer_data_is_stage_complete(dwhc_xfer_data_t *self)
{
    return self->packets == 0;
}

uint32_t dwhc_xfer_data_get_resultlen(dwhc_xfer_data_t *self)
{
    if (self->xfered > self->xfersize) {
        return self->xfersize;
    }

    return self->xfered;
}

usb_request_t *dwhc_xfer_data_get_urb(dwhc_xfer_data_t *self)
{
    return self->urb;
}

dwhc_scheduler_t *dwhc_xfer_data_get_scheduler(dwhc_xfer_data_t *self)
{
    if (!self->fsused) {
        return 0;
    }

    return &self->scheduler.base;
}

boolean dwhc_xfer_data_is_retry_ok(dwhc_xfer_data_t *self)
{
    return self->err_cnt <= MAX_BULK_TRIES;
}

usb_error_t dw2_xter_dtagedata_get_usb_err(dwhc_xfer_data_t *self)
{
    if (self->trstatus & DWHCI_HOST_CHAN_INT_STALL)
        return usb_err_stall;

    if (self->trstatus & DWHCI_HOST_CHAN_INT_XACT_ERROR)
        return usb_error_transaction;

    if (self->trstatus & DWHCI_HOST_CHAN_INT_BABBLE_ERROR)
        return usb_err_babble;

    if (self->trstatus & DWHCI_HOST_CHAN_INT_FRAME_OVERRUN)
        return usb_err_frame_overrun;

    if (self->trstatus & DWHCI_HOST_CHAN_INT_DATA_TOGGLE_ERROR)
        return usb_err_data_toggle;

    if (self->trstatus & DWHCI_HOST_CHAN_INT_AHB_ERROR)
        return usb_err_host_bus;

    return usb_err_unknown;
}

boolean dwhc_xfer_data_is_timeout(dwhc_xfer_data_t *self)
{
    if (self->timeout == USB_TIMEOUT_NONE)
        return false;

    return (jiffies - self->start) >= self->timeout ? true : false;
}
