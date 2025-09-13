//
// dwhciframeschednper.h
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
#ifndef INC_USB_DWC_NON_PERIODIC_H
#define INC_USB_DWC_NON_PERIODIC_H

#include <usb/dwhc_scheduler.h>
#include <types.h>

typedef struct dwhc_non_periodic {
    dw2_fsched_t    scheduler;
    unsigned        state;
    unsigned        tries;
#ifdef USE_USB_SOF_INTR
    uint16_t        offset;     ///< フレームオフセット
#endif
} dwhc_non_periodic_t;

void dwhc_non_periodic(dwhc_non_periodic_t *self);
void _dwhc_non_periodic(dw2_fsched_t *base);

void dwhc_non_periodic_start_split(dw2_fsched_t *base);
boolean dwhc_non_periodic_complete_split(dw2_fsched_t *base);
void dwhc_non_periodic_transaction_complete(dw2_fsched_t *base, uint32_t nStatus);

#ifndef USE_USB_SOF_INTR
void dwhc_non_periodic_wait_for_frame(dw2_fsched_t *base);
#else
uint16_t dwhc_non_periodic_get_frame_number(dw2_fsched_t *base);
void dwhc_non_periodic_delay(dw2_fsched_t *base, uint16_t offset);
#endif

boolean dwhc_non_periodic_is_odd_frame(dw2_fsched_t *base);

#endif
