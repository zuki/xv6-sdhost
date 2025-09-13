//
// dwhciframeschednsplit.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright(C) 2014  R. Stange <rsta2@o2online.de>
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
#include <usb/dwhc_scheduler.h>
#include <usb/dwhc_non_split.h>
#include <usb/dwhc_regs.h>
#include <types.h>
#include <arm.h>
#include <console.h>

#define FRAME_UNSET_NS (DWHCI_MAX_FRAME_NUMBER+1)

void dwhc_non_split(dwhc_non_split_t *self, boolean peiodic)
{
    assert(self != 0);

    dw2_fsched_t *base = (dw2_fsched_t *) self;

    base->_fscheduler = _dwhc_non_split;
    base->start_split = dwhc_non_split_start_split;
    base->complete_split = dwhc_non_split_complete_split;
    base->transaction_complete = dwhc_non_split_transaction_complete;
#ifndef USE_USB_SOF_INTR
    base->wait_for_frame = dwhc_non_split_wait_for_frame;
#else
    base->get_frame_number = dwhc_non_split_get_frame_number;
    base->periodic_delay = w2_fsched_nsplit_periodic_delay;
#endif
    base->is_odd_frame = dwhc_non_split_is_odd_frame;

    self->periodic = peiodic;
    self->next = FRAME_UNSET_NS;
}

void _dwhc_non_split(dw2_fsched_t *base)
{
    // nop
}

void dwhc_non_split_start_split(dw2_fsched_t *base)
{
    assert(0);
}

boolean dwhc_non_split_complete_split(dw2_fsched_t *base)
{
    assert(0);
    return false;
}

void dwhc_non_split_transaction_complete(dw2_fsched_t *base, uint32_t status)
{
    assert(0);
}

#ifndef USE_USB_SOF_INTR

void dwhc_non_split_wait_for_frame(dw2_fsched_t *base)
{
    dwhc_non_split_t *self = (dwhc_non_split_t *) base;

    uint32_t number;
    // 1. 現在のフレーム番号を取得
    number = get32(DWHCI_HOST_FRM_NUM);
    //uint32_t number_low = number & DWHCI_MAX_FRAME_NUMBER;
    // 2. 次のフレーム番号を取得
    self->next = (DWHCI_HOST_FRM_NUM_NUMBER(number)+1) & DWHCI_MAX_FRAME_NUMBER;
    // 3. 周期的でない場合, 次のフレームになるまで待つ
    // FIXME: 現状、いきなり番号を2つ以上飛んでしまう、または次のフレームに移行しないので条件が成立しない）
    if (!self->periodic) {
        number = get32(DWHCI_HOST_FRM_NUM);
        while ((DWHCI_HOST_FRM_NUM_NUMBER(number) & DWHCI_MAX_FRAME_NUMBER) != self->next) {
            // do nothing
        }
    }

}

#else

uint16_t dwhc_non_split_get_frame_number(dw2_fsched_t *base)
{
    dwhc_non_split_t *self = (dwhc_non_split_t *) base;

    uint32_t framnum = get32(DWHCI_HOST_FRM_NUM);
    self->next = (DWHCI_HOST_FRM_NUM_NUMBER(framnum+1) & DWHCI_MAX_FRAME_NUMBER;

    return serl->next;
}

void dwhc_non_split_periodic_delay(dw2_fsched_t *base, uint16_t offset)
{
	assert (0);
}

#endif

boolean dwhc_non_split_is_odd_frame(dw2_fsched_t *base)
{
    dwhc_non_split_t *self = (dwhc_non_split_t *) base;

    return self->next & 1 ? true : false;
}
