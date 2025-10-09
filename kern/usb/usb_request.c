//
// usb_request_.c
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
#include <usb/usb_request.h>
#include <types.h>
#include <console.h>
#include <slab.h>

struct slab_cache *URB;

void usb_request_init(void) {
    URB = slab_cache_create("urb", sizeof(usb_request_t), 64);
}

usb_request_t *usb_request_alloc(struct usb_endpoint *ep, const void *buffer, uint32_t buflen, usb_setup_data_t *setup_data)
{
    usb_request_t *urb = (usb_request_t *)slab_cache_alloc(URB);
    urb->ep            = ep;
    urb->setup_data    = setup_data;
    urb->buffer        = buffer;
    urb->buflen        = buflen;
    urb->status        = 0;
    urb->resultlen     = 0;
    urb->cb            = 0;
    urb->param         = 0;
    urb->ctx           = 0;
    urb->onnak         = false;
    urb->error         = usb_err_unknown;
    return urb;
}

void usb_reqeust_free(usb_request_t *self) {
    slab_cache_free(URB, self);
}

void usb_request_set_comp_cb(usb_request_t *self, usb_comp_cb *cb, void *param, void *ctx)
{
    trace("cb: 0x%p, param: 0x%p, ctx: 0x%p", cb, param, ctx);
    self->cb    = cb;
    self->param = param;
    self->ctx   = ctx;
}

void usb_request_call_comp_cb(usb_request_t *self)
{
    (*self->cb)(self, self->param, self->ctx);
}

void usb_request_debug(usb_request_t *self, int ba)
{
    cprintf("usb_request %d [0x%p] setup: [%c, %c, %u, %u, %u], buf: 0x%p, blen: %u, status: %d, rlen: %u, nak: %d, err: %d\n",
        ba, self, self->setup_data->reqtype, self->setup_data->req, self->setup_data->value,
        self->setup_data->index, self->setup_data->length, self->buffer,
        self->buflen, self->status, self->resultlen, self->onnak, self->error);
}
