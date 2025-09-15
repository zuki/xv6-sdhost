#include <usb/usb_function.h>
#include <usb/usb_device.h>
#include <usb/dwhc_device.h>
#include <usb/usb_endpoint.h>
#include <usb/usb_cfg_parser.h>
#include <types.h>
#include <console.h>
#include <mm.h>
#include <string.h>

void usb_function(usb_function_t *self, usb_dev_t *dev, usb_usb_cfg_parser_t *parser)
{
    self->configure = 0;
    self->dev = dev;
    self->usb_cfg_parser = (usb_usb_cfg_parser_t *)kmalloc(sizeof(usb_usb_cfg_parser_t));
    usb_cfg_parser_copy(self->usb_cfg_parser, parser);
    self->if_desc = (usb_if_desc_t *)usb_cfg_parser_get_curr_desc(self->usb_cfg_parser);
}

void usb_function_copy(usb_function_t *self, usb_function_t *func)
{
    self->configure = func->configure;
    self->dev = func->dev;
    self->usb_cfg_parser = (usb_usb_cfg_parser_t *)kmalloc(sizeof(usb_usb_cfg_parser_t));
    usb_cfg_parser_copy(self->usb_cfg_parser, func->usb_cfg_parser);
    self->if_desc = (usb_if_desc_t *)usb_cfg_parser_get_curr_desc(self->usb_cfg_parser);
}

void _usb_function(usb_function_t *self)
{
    self->if_desc = 0;
    _usb_cfg_paser(self->usb_cfg_parser);
    kmfree(self->usb_cfg_parser);
    self->usb_cfg_parser = 0;
    self->dev = 0;
    self->configure = 0;
}

boolean usb_function_init(usb_function_t *self)
{
    return true;
}

boolean usb_function_config(usb_function_t *self)
{
    assert(self->if_desc != 0);
    if (self->if_desc->alt != 0) {
        if (dwhc_control_message(self->dev->host,
            self->dev->ep0, REQUEST_OUT | REQUEST_TO_INTERFACE, SET_INTERFACE,
            self->if_desc->alt, self->if_desc->num, 0, 0) < 0) {
            return false;
        }
    }
    return true;
}

boolean usb_function_rescan_dev(usb_function_t *self)
{
    return false;
}

boolean usb_function_remove_dev(usb_function_t *self)
{
    return usb_dev_remove_dev(self->dev);
}

char *usb_function_get_if_name(usb_function_t *self)
{
    usb_if_desc_t *desc = self->if_desc;
    char *name = (char *)kmalloc(64);

    if (desc != 0 && desc->class != 0x00 && desc->class != 0xff) {
        sprintf(name, "int%x-%x-%x", desc->class, desc->subclass, desc->proto);
    } else {
        memmove(name, "unknown", 8);
    }
    info("func name=%s", name);

    return name;
}

uint8_t usb_function_get_num_eps(usb_function_t *self)
{
    return self->if_desc->neps;
}

boolean usb_function_select_if(usb_function_t *self, uint8_t class, uint8_t subclass, uint8_t proto)
{
    do {
        if (self->if_desc->class    == class
         && self->if_desc->subclass == subclass
         && self->if_desc->proto    == proto) {
            return true;
        }
        // 次のインタフェースにスキップ
        usb_dev_get_desc(self->dev, DESCRIPTOR_INTERFACE);
    } while ((self->if_desc = (usb_if_desc_t *)usb_function_get_desc(self, DESCRIPTOR_INTERFACE)) != 0);

    return false;
}

usb_dev_t *usb_function_get_dev(usb_function_t *self)
{
    return self->dev;
}

usb_endpoint_t *usb_function_get_ep0(usb_function_t *self)
{
    return self->dev->ep0;
}

dwhc_device_t *usb_function_get_host(usb_function_t *self)
{
    return self->dev->host;
}

const usb_desc_t *usb_function_get_desc(usb_function_t *self, uint8_t type)
{
    return usb_cfg_parser_get_desc(self->usb_cfg_parser, type);
}


uint8_t usb_function_get_if_num(usb_function_t *self)
{
    return self->if_desc->num;
}

uint8_t usb_function_get_if_class(usb_function_t *self)
{
    return self->if_desc->class;
}

uint8_t usb_function_get_if_subclass(usb_function_t *self)
{
    return self->if_desc->subclass;
}

uint8_t usb_function_get_if_proto(usb_function_t *self)
{
    return self->if_desc->proto;
}
