//
// dwhciframeschednper.c
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
#include <usb/dwhc_non_periodic.h>
#include <usb/dwhc_regs.h>
#include <usb/dwhc_xfer_data.h>
#include <types.h>
#include <arm.h>
#include <console.h>

typedef enum framescheduler_state{
    scheduler_state_start_split,
    scheduler_state_start_split_comp,
    scheduler_state_comp_split,
    scheduler_state_comp_retry,
    scheduler_state_comp_split_comp,
    scheduler_state_comp_split_failed,
    scheduler_state_unknown
} scheduler_state_t;

static void _dwhc_non_periodic(dwhc_scheduler_t *scheduler)
{
    dwhc_scheduler_free(scheduler);
}

static void dwhc_non_periodic_start_split(dwhc_scheduler_t *scheduler)
{
    dwhc_non_periodic_t *self = (dwhc_non_periodic_t *) scheduler;

#ifdef USE_USB_SOF_INTR
    if (self->state != scheduler_state_comp_split_failed)
        self->offset = 1;
#endif

    self->state = scheduler_state_start_split;
}

static boolean dwhc_non_periodic_complete_split(dwhc_scheduler_t *scheduler)
{
    dwhc_non_periodic_t *self =(dwhc_non_periodic_t *) scheduler;
    boolean result = false;

#ifdef USE_USB_SOF_INTR
    self->offset = 2;
#endif

    switch(self->state) {
    case scheduler_state_start_split_comp:
        self->state = scheduler_state_comp_split;
        self->tries = 3;
        result = true;
        break;

    case scheduler_state_comp_split:
    case scheduler_state_comp_retry:
#ifndef USE_USB_SOF_INTR
        delayus(5 * FRAME);
#else
        self->offset = 1;
#endif
        result = true;
        break;

    case scheduler_state_comp_split_comp:
    case scheduler_state_comp_split_failed:
        break;

    default:
        assert(0);
        break;
    }

    return result;
}

static void dwhc_non_periodic_transaction_complete(dwhc_scheduler_t *scheduler, uint32_t status)
{
    dwhc_non_periodic_t *self =(dwhc_non_periodic_t *) scheduler;

    switch(self->state)
    {
    case scheduler_state_start_split:
        assert(status & DWHCI_HOST_CHAN_INT_ACK);
        self->state = scheduler_state_start_split_comp;
        break;

    case scheduler_state_comp_split:
    case scheduler_state_comp_retry:
        if(status & DWHCI_HOST_CHAN_INT_XFER_COMPLETE) {
            self->state = scheduler_state_comp_split_comp;
        } else if(status &(DWHCI_HOST_CHAN_INT_NYET | DWHCI_HOST_CHAN_INT_ACK)) {
            if(self->tries-- == 0) {
#ifdef USE_USB_SOF_INTR
                self->offset = 1;
#endif
                self->state = scheduler_state_comp_split_failed;
            } else {
                self->state = scheduler_state_comp_retry;
            }
        } else if(status & DWHCI_HOST_CHAN_INT_NAK) {
#ifndef USE_USB_SOF_INTR
            delayus(5 * FRAME);
#else
            self->offset = 5;
#endif
            self->state = scheduler_state_comp_split_failed;

        } else {
            error("Invalid status 0x%x", status);
            assert(0);
        }
        break;

    default:
        assert(0);
        break;
    }
}

#ifndef USE_USB_SOF_INTR

static void dwhc_non_periodic_wait_for_frame(dwhc_scheduler_t *scheduler)
{
    // nop
}

#else

static uint16_t dwhc_non_periodic_get_frame_number(dwhc_scheduler_t *scheduler)
{
    dwhc_non_periodic_t *self =(dwhc_non_periodic_t *) scheduler;

    uint32_t framnum = get32(DWHCI_HOST_FRM_NUM);
    uint16_t fnum = DWHCI_HOST_FRM_NUM_NUMBER(framnum);

    assert (self->offset < 8);
	return (fnum + self->ofsser) & DWHCI_MAX_FRAME_NUMBER;
}

static void dwhc_non_periodic_delay(dwhc_scheduler_t *scheduler, uint16_t offset)
{
	assert (0);
}

#endif

static boolean dwhc_non_periodic_is_odd_frame(dwhc_scheduler_t *scheduler)
{
    return false;
}

static dwhc_scheduler_ops_t non_periodic_ops = {
    ._scheduler = _dwhc_non_periodic,
    .start_split = dwhc_non_periodic_start_split,
    .complete_split = dwhc_non_periodic_complete_split,
    .transaction_complete = dwhc_non_periodic_transaction_complete,
#ifndef USE_USB_SOF_INTR
    .wait_for_frame = dwhc_non_periodic_wait_for_frame,
#else
    .get_frame_number = dwhc_non_periodicget_frame_number,
    .periodic_delay = w2_scheduler_per_non_periodic_delay,
#endif
    .is_odd_frame = dwhc_non_periodic_is_odd_frame,
};

void dwhc_non_periodic(dwhc_non_periodic_t *self)
{
    self->scheduler.ops = &non_periodic_ops;
    self->scheduler.type = dwhc_scheduler_type_non_periodic;
    self->state = scheduler_state_unknown;
    self->tries = 0;
#ifdef USE_USB_SOF_INTR
    self->offset = 0;
#endif
}
