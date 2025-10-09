//
// dwhciframeschednsplit.h
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
#ifndef INC_USB_DWHC_NON_SPLIt_H
#define INC_USB_DWHC_NON_SPLIt_H

#include <usb/dwhc_scheduler.h>
#include <types.h>

typedef struct dwhc_non_split {
    dwhc_scheduler_t    scheduler;
    boolean             periodic;
    unsigned            next;
} dwhc_non_split_t;

void dwhc_non_split(dwhc_non_split_t *self);

#if 0
void _dwhc_non_split(dwhc_scheduler_t *scheduler);

void dwhc_non_split_start_split(dwhc_scheduler_t *scheduler);
boolean dwhc_non_split_complete_split(dwhc_scheduler_t *scheduler);
void dwhc_non_split_transaction_complete(dwhc_scheduler_t *scheduler, uint32_t status);

#ifndef USE_USB_SOF_INTR
void dwhc_non_split_wait_for_frame(dwhc_scheduler_t *scheduler);
#else
uint16_t dwhc_non_split_get_frame_number(dwhc_scheduler_t *scheduler);
void dwhc_non_split_periodic_delay(dwhc_scheduler_t *scheduler, uint16_t offset);
#endif

boolean dwhc_non_split_is_odd_frame(dwhc_scheduler_t *scheduler);
#endif

#endif
