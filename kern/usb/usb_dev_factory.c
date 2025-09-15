#include <usb/usb_dev_factory.h>
#include <types.h>
#include <string.h>
#include <mm.h>
#include <console.h>

// devices of usb classes
#include <usb/usb_standard_hub.h>
#include <usb/lan7800.h>
#include <usb/usb_cdcether.h>
#include <usb/usb_keyboard.h>

usb_function_t *usb_dev_factory_get_device(usb_function_t *parent, char *name)
{
    usb_function_t *result = 0;

    if (strcmp(name, "int9-0-0") == 0
     || strcmp(name, "int9-0-2") == 0) {            // USBハブ
        usb_standard_hub_t *dev = (usb_standard_hub_t *)kmalloc(sizeof(usb_standard_hub_t));
        usb_standardhub(dev, parent);
        result = (usb_function_t *)dev;
    } else if (strcmp(name, "ven424-7800") == 0) {  // CLAN7800 Ethernetコントローラ
        lan7800_t *dev = (lan7800_t *)kmalloc(sizeof(lan7800_t));
        lan7800(dev, parent);
        result = (usb_function_t *)dev;
    } else if (strcmp(name, "int2-6-0") == 0) {     // CDC Ethernetデバイス
        usb_cdcether_t *dev = (usb_cdcether_t *)kmalloc(sizeof(usb_cdcether_t));
        usb_cdcether(dev, parent);
        result = (usb_function_t *)dev;
    } else if (strcmp(name, "int3-1-1") == 0) {     // USBキーボード
        usb_keyboard_t *dev = (usb_keyboard_t *)kmalloc(sizeof(usb_keyboard_t));
        usb_keyboard(dev, parent);
        result = (usb_function_t *)dev;
    }

    if (result != 0)
        info("Using device/interface %s", name);

    kmfree(name);

    return result;
}
