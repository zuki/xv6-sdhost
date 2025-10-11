#include <usb/lan7800.h>
#include <usb/usb.h>
#include <usb/usb_request.h>
#include <usb/dwhc_device.h>
#include <usb/usb_dev_ns.h>
#include <usb/usb_endpoint.h>
#include <types.h>
#include <console.h>
#include <mm.h>
#include <mbox.h>
#include <clock.h>
#include <arm.h>
#include <string.h>
#include <net/net.h>
#include <net/ether.h>
#include <net/platform.h>

#define PRIV(x) ((lan7800_t *)(x)->priv)

// 前方参照
boolean lan7800_configure(usb_function_t *super);
static void lan7800_set_addr_filter(lan7800_t *self, int index, const uint8_t addr[MAC_ADDRESS_SIZE]);
static boolean lan7800_init_macaddr(lan7800_t *self);
static boolean lan7800_init_phy(lan7800_t *self);
static boolean lan7800_phy_write(lan7800_t *self, uint8_t index, uint16_t value);
static boolean lan7800_phy_read(lan7800_t *self, uint8_t index, uint16_t *phyvalue);
static boolean lan7800_wait_reg(lan7800_t *self, uint32_t index, uint32_t mask, uint32_t compare,
    uint32_t delay_micro, uint32_t timeout);
static boolean lan7800_readwrite_reg(lan7800_t *self, uint32_t index, uint32_t ormask, uint32_t andmask);
static boolean lan7800_write_reg(lan7800_t *self, uint32_t index, uint32_t value);
static boolean lan7800_read_reg(lan7800_t *self, uint32_t index, uint32_t *value);

void lan7800(lan7800_t *self, usb_function_t *func)
{
    usb_function_copy(&self->usb_func, func);
    //self->net_dev = (net_dev_t *)kmalloc(sizeof(net_dev_t));
    //assert(self->net_dev != 0);
    self->usb_func.configure = lan7800_configure;
    self->bulk_in = 0;
    self->bulk_out = 0;
    self->tx_buffer = (uint8_t *)kalloc(FRAME_BUFFER_SIZE+TX_HEADER_SIZE);
}

void _lan7800(lan7800_t *self)
{
    if (self->tx_buffer != 0) {
        kfree(self->tx_buffer);
        self->tx_buffer = 0;
    }

    if (self->bulk_out != 0) {
        usb_endpoint_free(self->bulk_out);
    }

    if (self->bulk_in != 0) {
        usb_endpoint_free(self->bulk_in);
    }

    _usb_function(&self->usb_func);
}

boolean lan7800_configure(usb_function_t *super)
{
    lan7800_t *self = (lan7800_t *)super;
    // USBコンフィグレーションをチェック
    if (usb_function_get_num_eps(&self->usb_func) != 3) {
        error("number of ep is not 3: %d", self->usb_func.if_desc->neps);
        return false;
    }

    if (usb_function_get_num_eps(&self->usb_func) != 3) {
        error("num of eps is not 3");
        return false;
    }

    const usb_ep_desc_t *ep_desc;

    while ((ep_desc = (usb_ep_desc_t *) usb_function_get_desc(&self->usb_func, DESCRIPTOR_ENDPOINT)) != 0) {
        if ((ep_desc->attr & 0x3F) == 0x02) {       // バルク転送
            if ((ep_desc->addr & 0x80) == 0x80) {   // 入力
                if (self->bulk_in != 0) {
                    error("bulk_in not null");
                    return false;
                }
                self->bulk_in = usb_endpoint_alloc(usb_function_get_dev(&self->usb_func), ep_desc);
            } else {                                    // 出力
                if (self->bulk_out != 0) {
                    error("bulk_out not null");
                    return false;
                }
                self->bulk_out = usb_endpoint_alloc(usb_function_get_dev(&self->usb_func), ep_desc);
            }
        }
    }

    if (self->bulk_in == 0 || self->bulk_out == 0) {
        error("ep bulk_in and bulk_out must exit");
        return false;
    }

    if (!usb_function_config(&self->usb_func)) {
        error("config usb_func failed");
        return false;
    }

    // チップIDをチェック
    uint32_t value;
    if (!lan7800_read_reg(self, ID_REV, &value)
     || (value & ID_REV_CHIP_ID_MASK) >> 16 != ID_REV_CHIP_ID_7800) {
        error("Invalid chip id (0x%x)", (value & ID_REV_CHIP_ID_MASK) >> 16);
        return false;
    }

    // ハードウェアを初期化

    // ハードウェアをリセット
    if (!lan7800_readwrite_reg(self, HW_CFG, HW_CFG_LRST, ~0U)
     || !lan7800_wait_reg(self, HW_CFG, HW_CFG_LRST, 0, 1000, 100)) {
        error("HW reset failed");
        return false;
    }
    // MACアドレスを初期化
    if (!lan7800_init_macaddr(self)) {
        error("Cannot init MAC address");
        return false;
    }

    // USB HS用
    if (!lan7800_write_reg(self, BURST_CAP, DEFAULT_BURST_CAP_SIZE / HS_USB_PKT_SIZE)
     || !lan7800_write_reg(self, BULK_IN_DLY, DEFAULT_BULK_IN_DELAY)) {
        error("failed for USB high speed")
        return false;
    }

    // LEDとSEFモードを有効化
    if (!lan7800_readwrite_reg(self, HW_CFG, HW_CFG_LED0_EN | HW_CFG_LED1_EN, ~HW_CFG_MEF)) {
        error("failed to enable the LEDs and SEF mode")
        return false;
    }

    // バーストCAPを有効に、RX FIFOがからの場合のNAKを無効に
    if (!lan7800_readwrite_reg(self, USB_CFG0, USB_CFG_BCE, ~USB_CFG_BIR)) {
        error("failed to enable burst CAP, disable NAK on RX FIFO empty")
        return false;
    }

    // FIFOサイズをセット
    if (!lan7800_write_reg(self, FCT_RX_FIFO_END, (MAX_RX_FIFO_SIZE - 512) / 512)
     || !lan7800_write_reg(self, FCT_TX_FIFO_END, (MAX_TX_FIFO_SIZE - 512) / 512)) {
        error("failed to set FIFO sizes");
        return false;
    }

    // 割り込みEPは使用しない
    if (!lan7800_write_reg(self, INT_EP_CTL, 0)
     || !lan7800_write_reg(self, INT_STS, INT_STS_CLEAR_ALL)) {
        error("failed not not to use interrupt EP");
        return false;
    }

    // フロー制御を無効化
    if (!lan7800_write_reg(self, FLOW, 0)
     || !lan7800_write_reg(self, FCT_FLOW, 0)) {
        error("failed to disable flow control");
        return false;
    }

    // 受信フィルタリングエンジンを初期化
    if (!lan7800_readwrite_reg(self, RFE_CTL, RFE_CTL_BCAST_EN | RFE_CTL_DA_PERFECT, ~0U)) {
        error("failed to init receive filtering engine");
        return false;
    }

    // PHYをリセット
    if (!lan7800_readwrite_reg(self, PMT_CTL, PMT_CTL_PHY_RST, ~0U)
     || !lan7800_wait_reg(self, PMT_CTL, PMT_CTL_PHY_RST | PMT_CTL_READY, PMT_CTL_READY, 1000, 100)) {
        error("failed to reset PHY");
        return false;
    }

    // AUTOネゴシエーションを有効化
    if (!lan7800_readwrite_reg(self, MAC_CR, MAC_CR_AUTO_DUPLEX | MAC_CR_AUTO_SPEED, ~0U)) {
        error("failed to enable AUTO negotiation");
        return false;
    }

    // TXを有効化
    if (!lan7800_readwrite_reg(self, MAC_TX, MAC_TX_TXEN, ~0U)
     || !lan7800_readwrite_reg(self, FCT_TX_CTL, FCT_TX_CTL_EN, ~0U)) {
        error("failed to enable TX");
        return false;
    }

    // RXを有効化
    if (!lan7800_readwrite_reg(self, MAC_RX, (MAX_RX_FRAME_SIZE << MAC_RX_MAX_SIZE_SHIFT) | MAC_RX_RXEN, ~MAC_RX_MAX_SIZE_MASK)
     || !lan7800_readwrite_reg(self, FCT_RX_CTL, FCT_RX_CTL_EN, ~0U)) {
        error("failed to enable RX");
        return false;
    }

    // PHYを初期化
    if (!lan7800_init_phy(self))
    {
        error("failed to init PHY");
        return false;
    }

    // USBデバイスとして登録
    usb_device_ns_add_dev(usb_device_ns_get(), "eth00", self, false);

    // FIXME: ネットデバイスとして登録
    //netdev_add_dev(self->net_dev);

    return true;
}

static ssize_t lan7800_send_frame(struct net_device *dev, const uint8_t *buf, size_t size)
{
    if (size > FRAME_BUFFER_SIZE)
        return false;

    lan7800_t *self = PRIV(dev);
    memmove(self->tx_buffer+TX_HEADER_SIZE, buf, size);
    *(uint32_t *)&self->tx_buffer[0] = (size & TX_CMD_A_LEN_MASK) | TX_CMD_A_FCS;
    *(uint32_t *)&self->tx_buffer[4] = 0;

    // USB転送（バルク）
    return dwhc_xfer(usb_function_get_host(&self->usb_func), self->bulk_out, self->tx_buffer, size+TX_HEADER_SIZE);
}

static ssize_t lan7800_receive_frame(struct net_device *dev, uint8_t *buf, size_t size)
{
    lan7800_t *self = PRIV(dev);

    usb_request_t *urb = usb_request_alloc(self->bulk_in, buf, (uint32_t)size, 0);

    if (!dwhc_submit_block_request(usb_function_get_host(&self->usb_func), urb, USB_TIMEOUT_NONE)) {
        usb_reqeust_free(urb);
        return -1;
    }

    uint32_t rlen = urb->resultlen;
    if (rlen < RX_HEADER_SIZE) {
        usb_reqeust_free(urb);
        return -1;
    }

    uint32_t status = *(uint32_t *) buf;    // RX command A
    if (status & RX_CMD_A_RED) {
        error("RX error (status 0x%X)", status);
        usb_reqeust_free(urb);
        return -1;
    }

    // CommandA(BC) + ethermet frame + FCS
    //                <- buf   ->
    size_t framelen = status & RX_CMD_A_LEN_MASK;
    if (framelen <= 4) {
        usb_reqeust_free(urb);
        return -1;
    }
    framelen -= 4;    // FCSは無視する

    debug("Frame received (status 0x%X)", status);

    memmove(buf, (uint8_t *)buf + RX_HEADER_SIZE, framelen); // RX コマンドA..Cを上書き

    usb_reqeust_free(urb);

    return framelen;
}

static int lan7800_is_linkup(struct net_device *dev)
{
    uint16_t status;
    lan7800_t *self = PRIV(dev);

    if (!lan7800_phy_read(self, 0x01, &status)) {
        return 0;
    }

    return status & (1 << 2);
}

link_speed_t lan7800_get_linkspeed(lan7800_t *self)
{
    // メインページレジスタを選択 (0-30): index 31に0を書き込む
    if (!lan7800_phy_write(self, 0x1F, 0))
        return link_speed_unknown;

    uint16_t status;
    if (!lan7800_phy_read(self, 0x1C, &status))
        return link_speed_unknown;

    // 自動ネゴシエーションされているかチェック
    assert (!(status & (1 << 14))); // 自動ネゴシエーションは有効でなければならない
    if (!(status & (1 << 15)))
        return link_speed_unknown;

    switch ((status >> 3) & 7) {
        case 0b000:     return link_speed_10half;
        case 0b001:     return link_speed_100half;
        case 0b010:     return link_speed_1000half;
        case 0b100:     return link_speed_10full;
        case 0b101:     return link_speed_100full;
        case 0b110:     return link_speed_1000full;
        default:        return link_speed_unknown;
    }
}

boolean lan7800_set_multicast_filter(lan7800_t *self, const uint8_t groups[][MAC_ADDRESS_SIZE])
{
    uint32_t rfe_ctl;
    if (!lan7800_read_reg(self, RFE_CTL, &rfe_ctl)) {
        error("failed to read RFE_CTL");
        return false;
    }

    rfe_ctl &= ~(RFE_CTL_UCAST_EN | RFE_CTL_MCAST_EN | RFE_CTL_DA_PERFECT | RFE_CTL_MCAST_HASH);
    rfe_ctl |= RFE_CTL_BCAST_EN;

    memset(self->filters, 0, sizeof(self->filters));

    for (int i = 0; groups[i][0]; i++) {
        if (i == 32)
            return false;
        lan7800_set_addr_filter(self, i+1, groups[i]);
        rfe_ctl |= RFE_CTL_DA_PERFECT;
    }

    for (int i = 1; i < NUM_OF_MAF; i++) {
        if (!lan7800_write_reg(self, MAF_HI(i), 0)
         || !lan7800_write_reg(self, MAF_LO(i), self->filters[i][1])
         || !lan7800_write_reg(self, MAF_HI(i), self->filters[i][0])) {
            return false;
        }
    }

    return lan7800_write_reg(self, RFE_CTL, rfe_ctl);
}

static void lan7800_set_addr_filter(lan7800_t *self, int index, const uint8_t addr[MAC_ADDRESS_SIZE])
{
    assert(0 < index && index < NUM_OF_MAF);

    uint32_t temp = addr[3];
    temp = addr[2] | (temp << 8);
    temp = addr[1] | (temp << 8);
    temp = addr[0] | (temp << 8);
    self->filters[index][1] = temp;

    temp = addr[5];
    temp = addr[4] | (temp << 8);
    temp |= MAF_HI_VALID | MAF_HI_TYPE_DST;
    self->filters[index][0] = temp;
}

static boolean lan7800_init_macaddr(lan7800_t *self)
{
    // MailBoxから取得
    if (!mbox_get_macaddr(self->macaddr)) {
        error("couldn't init MAC ADDRESS");
        return false;
    }

    uint32_t addr_low = (uint32_t) self->macaddr[0]
                 | ((uint32_t) self->macaddr[1] << 8)
                 | ((uint32_t) self->macaddr[2] << 16)
                 | ((uint32_t) self->macaddr[3] << 24);
    uint32_t addr_high = (uint32_t) self->macaddr[4]
                 | ((uint32_t) self->macaddr[5] << 8);

    if (!lan7800_write_reg(self, RX_ADDRL, addr_low)
     || !lan7800_write_reg(self, RX_ADDRH, addr_high)) {
        error("failed to write RX?ADDL/H");
        return false;
    }

    // 自身のMACアドレスのための完全フィルターエントリをセット
    if (!lan7800_write_reg(self, MAF_LO (0), addr_low)
     || !lan7800_write_reg(self, MAF_HI (0), addr_high | MAF_HI_VALID)) {
        error("failed to write MAF_LO/HI");
        return false;
    }

    info("MAC address is %02x:%02x:%02x:%02x:%02x:%02x",
        self->macaddr[0], self->macaddr[1], self->macaddr[2],
        self->macaddr[3], self->macaddr[4], self->macaddr[5]);

    return true;
}

static boolean lan7800_init_phy(lan7800_t *self)
{
    // メインページレジスタを選択 (0-30)
    if (!lan7800_phy_write(self, 0x1F, 0)) {
        error("failed to select main page reg");
        return false;
    }


    // LEDデフォルトを変更:
    //      orange = link1000/activity
    //      green  = link10/link100/activity
    // led: 0=link/activity          1=link1000/activity
    //      2=link100/activity       3=link10/activity
    //      4=link100/1000/activity  5=link10/1000/activity
    //      6=link10/100/activity    14=off    15=on
    uint16_t modesel;
    if (!lan7800_phy_read(self, 0x1D, &modesel)) {
        error("failed to read default LED");
        return false;
    }

    modesel &= ~0xFF;
    modesel |= 1 << 0;
    modesel |= 6 << 4;

    return lan7800_phy_write(self, 0x1D, modesel);
}

static boolean lan7800_phy_write(lan7800_t *self, uint8_t index, uint16_t value)
{
    if (!lan7800_wait_reg (self, MII_ACC, MII_ACC_MII_BUSY, 0, 0, 100)
     || !lan7800_write_reg(self, MII_DATA, value)) {
        error("failed to write NII_DATA with %d", value);
        return false;
     }

    // アドレス、インデックス、方向をセット（PHYへ書き込み）
    uint32_t mii_access  = (PHY_ADDRESS << MII_ACC_PHY_ADDR_SHIFT) & MII_ACC_PHY_ADDR_MASK;
    mii_access |= ((uint32_t) index << MII_ACC_MIIRINDA_SHIFT) & MII_ACC_MIIRINDA_MASK;
    mii_access |= MII_ACC_MII_WRITE | MII_ACC_MII_BUSY;

    if (!lan7800_write_reg(self, MII_ACC, mii_access)) {
        error("failed to write NII_ACC with %x", mii_access);
        return false;
    }

    return lan7800_wait_reg(self, MII_ACC, MII_ACC_MII_BUSY, 0, 0, 100);
}

static boolean lan7800_phy_read(lan7800_t *self, uint8_t index, uint16_t *phyvalue)
{
    if (!lan7800_wait_reg(self, MII_ACC, MII_ACC_MII_BUSY, 0, 0, 100)) {
        error("MII_ACC is busy");
        return false;
    }

    // アドレス、インデックス、方向をセット（PHYから読み込み）
    uint32_t mii_access  = (PHY_ADDRESS << MII_ACC_PHY_ADDR_SHIFT) & MII_ACC_PHY_ADDR_MASK;
    mii_access |= ((uint32_t) index << MII_ACC_MIIRINDA_SHIFT) & MII_ACC_MIIRINDA_MASK;
    mii_access |= MII_ACC_MII_READ | MII_ACC_MII_BUSY;

    uint32_t value;
    if (!lan7800_write_reg(self, MII_ACC, mii_access)
     || !lan7800_wait_reg (self, MII_ACC, MII_ACC_MII_BUSY, 0, 0, 100)
     || !lan7800_read_reg(self, MII_DATA, &value)) {
        error("failed to read MII_DATA");
        return false;
    }
    *phyvalue = value & MII_DATA_MASK;
    return true;
}

static boolean lan7800_wait_reg(lan7800_t *self, uint32_t index, uint32_t mask, uint32_t compare,
    uint32_t delay_micro, uint32_t timeout)
{
    uint64_t start_hz = get_ticks();
    uint32_t value;
    do {
        if (delay_micro > 0)
            delayus(delay_micro);

        if (get_ticks() - start_hz >= timeout)
            return false;

        if (!lan7800_read_reg(self, index, &value))
            return false;

    } while ((value & mask) != compare);

    return true;
}

static boolean lan7800_readwrite_reg (lan7800_t *self, uint32_t index, uint32_t ormask, uint32_t andmask)
{
    uint32_t value;
    if (!lan7800_read_reg(self, index, &value))
        return false;

    value &= andmask;
    value |= ormask;

    return lan7800_write_reg(self, index, value);
}

static boolean lan7800_write_reg(lan7800_t *self, uint32_t index, uint32_t value)
{
    if (dwhc_control_message(
            usb_function_get_host(&self->usb_func),
            usb_function_get_ep0(&self->usb_func),
            REQUEST_OUT | REQUEST_VENDOR, WRITE_REGISTER,
            0, index, &value, sizeof value) < 0) {
        warn("Cannot write register 0x%x", index);
        return false;
    }

    return true;
}

static boolean lan7800_read_reg(lan7800_t *self, uint32_t index, uint32_t *value)
{
    if (dwhc_control_message(
            usb_function_get_host(&self->usb_func),
            usb_function_get_ep0(&self->usb_func),
            REQUEST_IN | REQUEST_VENDOR, READ_REGISTER,
            0, index, value, sizeof *value) != (int) sizeof *value) {
        warn("Cannot read register 0x%x", index);
        return false;
    }

    return true;
}

static int lan7800_net_open(struct net_device *dev)
{
    return 0;
}

static int lan7800_net_close(struct net_device *dev)
{
    return 0;
}

static int lan7800_net_transmit(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst)
{
    return ether_transmit_helper(dev, type, data, len, dst, lan7800_send_frame);
}

struct net_device_ops lan7800_net_ops = {
    .open = lan7800_net_open,
    .close = lan7800_net_close,
    .transmit = lan7800_net_transmit,
    .linkup = lan7800_is_linkup,
};

int lan7800_net_init(lan7800_t *self)
{
    struct net_device *dev;

        // setup device driver structure
    dev = net_device_alloc();
    if (!dev) {
        error("net_device_alloc() failure");
        return -1;
    }
    ether_setup_helper(dev);

    memcpy(dev->addr, self->macaddr, sizeof(dev->addr));
    dev->priv = self;
    //memcpy(dev->name, "eth00", sizeof("eth00"));
    dev->ops = &lan7800_net_ops;
    if (net_device_register(dev) == -1) {
        error("net_device_register() failure");
        memory_free(dev);
        return -1;
    }
    self->net_dev = dev;

    return 0;
}

void lan7800_net_handler(void)
{
    struct net_device *dev = net_device_by_name("net0");
    if (!dev) return;

    if (ether_input_helper(dev, lan7800_receive_frame) == 0)
        intr_raise_irq(INTR_IRQ_SOFTIRQ);
}
