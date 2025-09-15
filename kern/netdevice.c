#include <netdevice.h>
#include <types.h>

const char *Speed_String[net_dev_speed_unknown] = {
    "10BASE-T Half-duplex",
    "10BASE-T Full-duplex",
    "100BASE-TX Half-duplex",
    "100BASE-TX Full-duplex",
    "1000BASE-T Half-duplex",
    "1000BASE-T Full-duplex"
};

static uint32_t number = 0;

static net_dev_t *devices[MAX_NET_DEVICES];

void netdev_add_dev(net_dev_t *self)
{
    if (number < MAX_NET_DEVICES) {
        devices[number++] = self;
    }
}

const char *netdev_get_speedstr(net_dev_t *self, net_dev_speed_t speed)
{
    if (speed >= net_dev_speed_unknown)
        return "Unknown";

    return Speed_String[speed];
}

net_dev_t *netdev_get_dev(net_dev_t *self, unsigned num)
{
    if (num < number) {
        return devices[num];
    }

    return 0;
}

net_dev_t *netdev_get_dev_type(net_dev_t *self, net_dev_type_t type)
{
    for (int num = 0; num < number; num++) {
        net_dev_t *dev = devices[num];
        if (dev == 0)
            break;
        if (type == net_dev_any || self->type == type) {
            return dev;
        }
    }

    return 0;
}
