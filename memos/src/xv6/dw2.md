# linuxのドライバを調査

## dw2ドライバ (`drivers/usb/dwc2`)

```c
static inline u32 dwc2_read_core_intr(struct dwc2_hsotg *hsotg)
{
    return dwc2_readl(hsotg, GINTSTS) & dwc2_readl(hsotg, GINTMSK);
}
```

- hcd_intr.c

```c
/* This function handles interrupts for the HCD */
irqreturn_t dwc2_handle_hcd_intr(struct dwc2_hsotg *hsotg)
{
    u32 gintsts, dbg_gintsts;
    irqreturn_t retval = IRQ_NONE;

    spin_lock(&hsotg->lock);
    /* Check if HOST Mode */
    if (dwc2_is_host_mode(hsotg)) {
        gintsts = dwc2_read_core_intr(hsotg);
        if (gintsts & GINTSTS_SOF)      // [3] SOF, マイクロSOF, Keep-AliveがUSBで送信された
            dwc2_sof_intr(hsotg);
        if (gintsts & GINTSTS_RXFLVL)   // [4] RxFIFOにパケットが少なくとも1つある
            dwc2_rx_fifo_level_intr(hsotg);
        if (gintsts & GINTSTS_NPTXFEMP) // [5] 非周期的TxFIFOに書き込みOK
            dwc2_np_tx_fifo_empty_intr(hsotg);
        if (gintsts & GINTSTS_PRTINT)   // [24] ポートステータスに変化あり
            dwc2_port_intr(hsotg);
        if (gintsts & GINTSTS_HCHINT)   // [25] いずれかのチャネルに割り込みあり
            dwc2_hc_intr(hsotg);
        if (gintsts & GINTSTS_PTXFEMP)  // [26] 周期的TxFIFOに書き込みOK
            dwc2_perio_tx_fifo_empty_intr(hsotg);
    }
    spin_unlock(&hsotg->lock);
    return retval;
}

/*
 * Handles the Rx FIFO Level Interrupt, which indicates that there is
 * at least one packet in the Rx FIFO. The packets are moved from the FIFO to
 * memory if the DWC_otg controller is operating in Slave mode.
 */
static void dwc2_rx_fifo_level_intr(struct dwc2_hsotg *hsotg)
{
    u32 grxsts, chnum, bcnt, dpid, pktsts;
    struct dwc2_host_chan *chan;

    grxsts = dwc2_readl(hsotg, GRXSTSP);    // RxFIFOの先頭を返す
    chnum = (grxsts & 0xf) >> 0;
    chan = hsotg->hc_ptr_array[chnum];
    if (!chan) {
        dev_err(hsotg->dev, "Unable to get corresponding channel\n");
        return;
    }

    bcnt = (grxsts & (0x7ff << 4)) >> 4;    // 受信したINデータパケットのバイト数
    dpid = (grxsts & (0x3 << 15)) >> 15;    // 受信パケットのデータPID
    pktsts = (grxsts & (0xf << 17)) >> 17;  // 受信パケットのステータス

    switch (pktsts) {
    case GRXSTS_PKTSTS_HCHIN:   // INデータパケット
        /* データを干すのバッファに読み込む */
        if (bcnt > 0) {
            dwc2_read_packet(hsotg, chan->xfer_buf, bcnt);

            /* 次に受け取るパケットのためにHCフィールドを更新する */
            chan->xfer_count += bcnt;
            chan->xfer_buf += bcnt;
        }
        break;
    case GRXSTS_PKTSTS_HCHIN_XFER_COMP: // IN転送が完了
    case GRXSTS_PKTSTS_DATATOGGLEERR:   // データトグルエラー
    case GRXSTS_PKTSTS_HCHHALTED:       // チャネル停止
        /* 割り込みで処理されるのでデータは単に無視する */
        break;
    default:
        dev_err(hsotg->dev,
            "RxFIFO Level Interrupt: Unknown status %d\n", pktsts);
        break;
    }
}
```

- hcd.c

```c
void dwc2_read_packet(struct dwc2_hsotg *hsotg, u8 *dest, u16 bytes)
{
    u32 *data_buf = (u32 *)dest;
    int word_count = (bytes + 3) / 4;
    int i;

    for (i = 0; i < word_count; i++, data_buf++)
        *data_buf = dwc2_readl(hsotg, HCFIFO(0));   // ホストINチャネル0のDFIFO Read
}
```

## lan78xxドライバ (`drivers/net/usb/lan78xx.c)

```c
static const struct net_device_ops lan78xx_netdev_ops = {
    .ndo_open       = lan78xx_open,
    .ndo_stop       = lan78xx_stop,
    .ndo_start_xmit     = lan78xx_start_xmit,
    .ndo_tx_timeout     = lan78xx_tx_timeout,
    .ndo_change_mtu     = lan78xx_change_mtu,
    .ndo_set_mac_address    = lan78xx_set_mac_addr,
    .ndo_validate_addr  = eth_validate_addr,
    .ndo_eth_ioctl      = phy_do_ioctl_running,
    .ndo_set_rx_mode    = lan78xx_set_multicast,
    .ndo_set_features   = lan78xx_set_features,
    .ndo_vlan_rx_add_vid    = lan78xx_vlan_rx_add_vid,
    .ndo_vlan_rx_kill_vid   = lan78xx_vlan_rx_kill_vid,
    .ndo_features_check = lan78xx_features_check,
};

lan78xx_start_xmit(struct sk_buff *skb, struct net_device *net)
{
    struct lan78xx_net *dev = netdev_priv(net);
    unsigned int tx_pend_data_len;

    if (test_bit(EVENT_DEV_ASLEEP, &dev->flags))
        schedule_delayed_work(&dev->wq, 0);

    skb_tx_timestamp(skb);

    lan78xx_tx_pend_skb_add(dev, skb, &tx_pend_data_len);

    /* Set up a Tx URB if none is in progress */

    if (skb_queue_empty(&dev->txq))
        napi_schedule(&dev->napi);

    /* Stop stack Tx queue if we have enough data to fill
     * all the free Tx URBs.
     */
    if (tx_pend_data_len > lan78xx_tx_urb_space(dev)) {
        netif_stop_queue(net);

        netif_dbg(dev, hw, dev->net, "tx data len: %u, urb space %u",
              tx_pend_data_len, lan78xx_tx_urb_space(dev));

 /* Kick off transmission of pending data */

        if (!skb_queue_empty(&dev->txq_free))
            napi_schedule(&dev->napi);
    }

    return NETDEV_TX_OK;
}
```
