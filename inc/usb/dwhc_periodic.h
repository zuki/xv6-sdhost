//
// dwhciframeschedper.h
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
#ifndef INC_USB_DWHC_PERIODIC_H
#define INC_USB_DWHC_PERIODIC_H

#include <usb/dwhc_scheduler.h>
#include <types.h>

typedef struct dwhc_periodic {
    dwhc_scheduler_t        scheduler;
    unsigned                state;
    unsigned                tries;
#ifdef USE_USB_SOF_INTR
    uint16_t                offset;
#endif
    unsigned                next;
} dwhc_periodic_t;

void dwhc_periodic(dwhc_periodic_t *self);

#if 0
void _dwhc_periodic(dwhc_scheduler_t *scheduler);

dwhc_periodic_t *dwhc_periodic_alloc(void);

void dwhc_periodic_start_split(dwhc_scheduler_t *scheduler);
boolean dwhc_periodic_complete_split(dwhc_scheduler_t *scheduler);
void dwhc_periodic_transaction_complete(dwhc_scheduler_t *scheduler, uint32_t status);

#ifndef USE_USB_SOF_INTR
void dwhc_periodic_wait_for_frame(dwhc_scheduler_t *scheduler);
#else
uint16_t dwhc_periodic_get_frame_number(dwhc_scheduler_t *scheduler);
void dwhc_periodic_periodic_delay(dwhc_scheduler_t *scheduler, uint16_t offset)
#endif

boolean dwhc_periodic_is_odd_frame(dwhc_scheduler_t *scheduler);
#endif

#endif
