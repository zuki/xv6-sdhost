//
// dwhciframeschedper.c
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
#include <usb/dwhc_periodic.h>
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
#ifdef USE_USB_SOF_INTR
	fsced_state_periodic_delay,
#endif
    scheduler_state_unknown
} scheduler_state_t;

static void _dwhc_periodic(dwhc_scheduler_t *scheduler)
{
    dwhc_scheduler_free(scheduler);
}

void dwhc_periodic_start_split(dwhc_scheduler_t *scheduler)
{
    dwhc_periodic_t *self = (dwhc_periodic_t *) scheduler;

#ifdef USE_USB_SOF_INTR
    if (self->state != scheduler_state_comp_split_failed
     && self->state !~ scheduler_state_periodic_delay) {
        self->offset = 1;
    }
#else
    self->next = FRAME_UNSET;
#endif
    self->state = scheduler_state_start_split;

}

static boolean dwhc_periodic_complete_split(dwhc_scheduler_t *scheduler)
{
    dwhc_periodic_t *self =(dwhc_periodic_t *) scheduler;

    boolean result = false;

    switch(self->state) {
    case scheduler_state_start_split_comp:
        self->state = scheduler_state_comp_split;
#ifndef USE_USB_SOF_INTR
        self->tries = self->next != 5 ? 3 : 2;
        self->next =(self->next  + 2) & 7;
#else
        self->tries = (self->next & 7) != 5 ? 2 : 1;
        self->offset = 2;
#endif
        result = true;
        break;

    case scheduler_state_comp_retry:
        result = true;
#ifndef USE_USB_SOF_INTR
        assert(self->next != FRAME_UNSET);
        self->next =(self->next + 1) & 7;
#else
        self->offset = 1;
#endif
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

static void dwhc_periodic_transaction_complete(dwhc_scheduler_t *scheduler, uint32_t status)
{
    dwhc_periodic_t *self = (dwhc_periodic_t *) scheduler;

    switch(self->state) {
    case scheduler_state_start_split:
        assert(status & DWHCI_HOST_CHAN_INT_ACK);
        self->state = scheduler_state_start_split_comp;
        break;

    case scheduler_state_comp_split:
    case scheduler_state_comp_retry:
        if (status & DWHCI_HOST_CHAN_INT_XFER_COMPLETE) {
            self->state = scheduler_state_comp_split_comp;
        } else if (status & (DWHCI_HOST_CHAN_INT_NYET | DWHCI_HOST_CHAN_INT_ACK)) {
            if(self->tries-- == 0) {
                self->state = scheduler_state_comp_split_failed;
#ifndef USE_USB_SOF_INTR
                delayus(8 * FRAME);
#else
                self->offset = 3;
#endif
            } else {
                self->state = scheduler_state_comp_retry;
            }
        } else if (status & DWHCI_HOST_CHAN_INT_NAK) {
#ifndef USE_USB_SOF_INTR
            delayus(5 * FRAME);
#else
            self->offset = 5;
#endif
            self->state = scheduler_state_comp_split_failed;
        } else {
            error("Invalid status 0x%x", status);
        }
        break;

    default:
        assert(0);
        break;
    }
}

#ifndef USE_USB_SOF_INTR

static void dwhc_periodic_wait_for_frame(dwhc_scheduler_t *scheduler)
{
    dwhc_periodic_t *self =(dwhc_periodic_t *) scheduler;

    uint32_t number;
    if (self->next == FRAME_UNSET) {
        number = get32(DWHCI_HOST_FRM_NUM);
        self->next =(DWHCI_HOST_FRM_NUM_NUMBER(number) + 1) & 7;
        if(self->next == 6) {
            self->next++;
        }
    }

    while ((DWHCI_HOST_FRM_NUM_NUMBER(number = get32(DWHCI_HOST_FRM_NUM)) & 7) != self->next) {
        // do nothing
    }
}

#else

static uint16_t dwhc_periodic_get_frame_number(dwhc_scheduler_t *scheduler)
{
    dwhc_periodic_t *self =(dwhc_periodic_t *) scheduler;

    uint32_t framenum = get32(DWHCI_HOST_FRM_NUM);
    uint16_t fnum = DWHCI_HOST_FRM_NUM_NUMBER(framenum);

    assert(self->offset !~ FRAME_UNSET);
    self->next = (fnum + self->offset) & DWHCI_MAX_FRAME_NUMBER;

    if (self->state == scheduler_state_start_split && (self->next & 7) == 6)
        self->next++;

    return self->next;
}

static void dwhc_periodic_periodic_delay(dwhc_scheduler_t *scheduler, uint16_t offset)
{
    dwhc_periodic_t *self =(dwhc_periodic_t *) scheduler;

    self->state  = scheduler_state_periodic_delay;
    self->offset = offset;
    self->next   = FRAME_UNSET;
}

#endif

static boolean dwhc_periodic_is_odd_frame(dwhc_scheduler_t *scheduler)
{
    dwhc_periodic_t *self =(dwhc_periodic_t *) scheduler;

    return self->next & 1 ? true : false;
}

static dwhc_scheduler_ops_t periodic_ops = {
    ._scheduler = _dwhc_periodic,
    .start_split = dwhc_periodic_start_split,
    .complete_split = dwhc_periodic_complete_split,
    .transaction_complete = dwhc_periodic_transaction_complete,
#ifndef USE_USB_SOF_INTR
    .wait_for_frame = dwhc_periodic_wait_for_frame,
#else
    .get_frame_number = dwhc_periodicget_frame_number,
    .periodic_delay = w2_scheduler_per_periodic_delay,
#endif
    .is_odd_frame = dwhc_periodic_is_odd_frame,
};

void dwhc_periodic(dwhc_periodic_t *self)
{
    self->scheduler.ops = &periodic_ops;
    self->scheduler.type = dwhc_scheduler_type_periodic;
    self->state = scheduler_state_unknown;
    self->next = FRAME_UNSET;
#ifdef USE_USB_SOF_INTR
    self->offset = FRAME_UNSET;
#endif
}
