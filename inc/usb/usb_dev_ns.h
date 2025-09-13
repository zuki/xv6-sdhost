//
// devicenameservice.h
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
#ifndef INC_USB_DEV_NS_H
#define INC_USB_DEV_NS_H

#include <types.h>

typedef struct usb_device_info {
    struct usb_device_info *next;
    char            *name;
    void            *dev;
    boolean          blkdev;
} usb_dev_info_t;

typedef struct usb_deviceice_ns {
    usb_dev_info_t  *list;
} usb_device_ns_t;

void usb_device_ns(usb_device_ns_t *self);
void _usb_devicee_ns(usb_device_ns_t *self);

void usb_device_ns_add_dev(usb_device_ns_t *self, const char *name, void *dev, boolean blkdev);

void *usb_device_ns_get_dev(usb_device_ns_t *self, const char *name, boolean blkdev);

usb_device_ns_t *usb_device_ns_get(void);

#endif
