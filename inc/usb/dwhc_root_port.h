#ifndef INC_USB_DWHC_ROOT_PORT_H
#define INC_USB_DWHC_ROOT_PORT_H

#include <usb/usb_device.h>
#include <types.h>

struct dwhc_device;
struct usb_device;

typedef struct dwhc_root_port {
    struct dwhc_device   *host;
    struct usb_device  *dev;
} dwhc_root_port_t;

void dwhc_root_port(dwhc_root_port_t *self, struct dwhc_device *host);
void _dwhc_root_port(dwhc_root_port_t *self);
boolean dwhc_root_port_init(dwhc_root_port_t *self);
boolean dwhc_root_port_rescan_dev(dwhc_root_port_t *self);
boolean dwhc_root_port_remove_dev(dwhc_root_port_t *self);
void dwhc_root_port_handle_port_status_change(dwhc_root_port_t *self);
void dwhc_root_port_port_status_changed(dwhc_root_port_t *self);

#endif
