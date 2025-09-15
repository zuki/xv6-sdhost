//
// dwhciframescheduler.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_DWHC_SCHEDULER_H
#define INC_USB_DWHC_SCHEDULER_H

#include <types.h>

#define FRAME           125        // micro seconds
#define FRAME_UNSET     8

typedef struct dwhc_scheduler {
    void (*_scheduler)(struct dwhc_scheduler *self);

    void (*start_split)(struct dwhc_scheduler *self);
    boolean (*complete_split)(struct dwhc_scheduler *self);
    void (*transaction_complete)(struct dwhc_scheduler *self, uint32_t status);
#ifndef USE_USB_SOF_INTR
    void (*wait_for_frame)(struct dwhc_scheduler *self);
#else
    uint16_t (*get_frame_number)(struct dwhc_scheduler *base);
    void (*periodic_delay)(struct dwhc_scheduler *base, uint16_t offset);
#endif
    boolean (*is_odd_frame)(struct dwhc_scheduler *self);
} dwhc_scheduler_t;

#endif
