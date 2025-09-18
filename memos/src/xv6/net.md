# Network (Ethernet) 機能を導入

## circleのnetサブシステムの調査

### 構造

- 基幹
    - netsubsystem.cpp, netconfig.cpp   => net.c, net/net_config.c
- 物理層
    - netdevlayer.cpp, lan7800.cpp, usbcdcethernet.cpp  => net/internet.c, usb/lan7800.c, usb/usb/usb_cdc_ether.c
- リンク層
    - linklayer.cpp, arphandler.cpp => net/link_layer.c, net/arp_handler.c
- インターネット層
    - networklayer.cpp, icmphandler.cpp => net/internet_layer.c, net/icmp_handler.c
- トランスポート層
    - transportlayer.cpp, netconnection.cpp, udpconnection.cpp, tcpconnection.cpp => net/transport_layer.c, udp_connection.c, tcp_connection.c
- アプリケーション層
    - dhcpclient.cpp, dnsclientcpp, httpclient.cp, httpdaemon.cpp, ntpclient.cpp, ntpdaemon.cpp, tftpdaemon.cpp
- ライブラリ
    - netsocket.cpp, socket.cpp,  => net/socket.c

### 受信処理

- 受信処理はカーネルスレッドで実行される
- netdevlayerで受信されたデータがTCP/IPの階層を上に伝わっていく
- アプリケーション（実際はsocket()）は空のpBufferを用意してtransportlayer#Receive()を実行する

```cpp
CNetTask::Run() {
  while(1) {
    netsubsystem#Process();
    scheduler#Yield();
  }
}
```
#### 一度の受信処理の関数呼び出しシーケンス

```cpp
CNetSubSystem::Process() {
  netdevlayer.Process();      // ethernetパケット取得
  linklayer.Process();        // ipパケットを取り出す
  networklayer.Process();     // tcp/udpパケットを取り出す
  transportlayer.Process();   // データを取り出す
}
```

#### 各レイヤのProcess()関数

**netdevlayer**

このレイヤは実際にデータの送受信を行う

```cpp
void CNetDeviceLayer::Process (void) {
    // 1. ネットワークデバイスの設定
    m_pDevice = CNetDevice::GetNetDevice (m_DeviceType);
    // 2. タスクPHYTaskの作成
    new CPHYTask (m_pDevice);
    // 3. バッファの作成
    DMA_BUFFER (u8, Buffer, FRAME_BUFFER_SIZE);
    // 4. 送信: lan7800/usb_cdd_etherのSendFrame()関数が呼び出されてEthernetパケットを送信
    while (m_TxQueue.Dequeue(Buffer) > 0) {
        m_pDevice->SendFrame (Buffer, nLength);
    }
    // 5. 受信: lan7800/usb_cdd_etherのReceiveFrame()関数を呼び出してEthernetパケットを
    //    受信してRxQueueに入れる
    while (m_pDevice->ReceiveFrame (Buffer, &nLength)) {
        m_RxQueue.Enqueue (Buffer, nLength);
    }
}
```

**linklayer**

```cpp
void CLinkLayer::Process (void)
{
    // 1. 自身のMACアドレスを取得
    const CMACAddress *pOwnMACAddress = m_pNetDevLayer->GetMACAddress ();
    // 2. バッファを作成
    u8 Buffer[FRAME_BUFFER_SIZE];
    // 3. RxQueueからEthernetパケットを取り出す
    while (m_pNetDevLayer->Receive (Buffer, &nLength)) {
        // 4. Ethernetヘッダーを読み込む
        TEthernetHeader *pHeader = (TEthernetHeader *) Buffer;
        // 5. 宛先が自分宛てでなければ無視する
        CMACAddress MACAddressReceiver (pHeader->MACReceiver);
        if (    MACAddressReceiver != *pOwnMACAddress
            && !MACAddressReceiver.IsBroadcast ()) {
            if (!MACAddressReceiver.IsMulticast ()) continue;
            for (unsigned i = 0; i < MaxGroups; i++) {
                if (m_MulticastGroup[i] == MACAddressReceiver) break;
            }
            if (i == MaxGroups) continue;
        }
        // 6. 自分宛てであればEthernetヘッダーを取り除いて対象のプロトコルのRxQueueにエンキュー
        switch (pHeader->nProtocolType) {
        case BE (ETH_PROT_IP):
            m_IPRxQueue.Enqueue (Buffer+sizeof (TEthernetHeader), nLength);
            break;
        case BE (ETH_PROT_ARP):
            m_ARPRxQueue.Enqueue (Buffer+sizeof (TEthernetHeader), nLength);
            break;
        default:
            ...
            break;
        }
    }
    // 7. ARPデータの処理
    m_pARPHandler->Process ();
}
```

**networaklayer**

```cpp
void CNetworkLayer::Process (void) {
    // 1. 自身のIPアドレスを取得
    const CIPAddress *pOwnIPAddress = m_pNetConfig->GetIPAddress ();
    // 2. バッファを作成
    u8 Buffer[FRAME_BUFFER_SIZE];
    // 3. IPRxQueueからデータを取得
    while (m_pLinkLayer->Receive (Buffer, &nResultLength)) {
        // 4. IPヘッダーを取り出す
        TIPHeader *pHeader = (TIPHeader *) Buffer;
        CIPAddress IPAddressDestination (pHeader->DestinationAddress);
        // 5. 自分宛てでなければ無視
        if (   *pOwnIPAddress != IPAddressDestination
            && !IPAddressDestination.IsBroadcast ()
            && *m_pNetConfig->GetBroadcastAddress () != IPAddressDestination
            && !IPAddressDestination.IsMulticast ()) continue;
        // 6. ipヘッダーの情報を取得
        pParam->nProtocol = pHeader->nProtocol;
        memcpy (pParam->SourceAddress, pHeader->SourceAddress, IP_ADDRESS_SIZE);
        memcpy (pParam->DestinationAddress, pHeader->DestinationAddress, IP_ADDRESS_SIZE);
        // 7. 対応するプロトコルのRxQueueにエンキュー
        if (pHeader->nProtocol == IPPROTO_ICMP) {
            m_ICMPRxQueue.Enqueue (Buffer+nHeaderLength, nResultLength, pParam);
        } else if (pHeader->nProtocol == IPPROTO_IGMP) {
            m_IGMPRxQueue.Enqueue (Buffer+nHeaderLength, nResultLength, pParam);
        } else {
            m_RxQueue.Enqueue (Buffer+nHeaderLength, nResultLength, pParam);
        }
    }
    // 8. ICMPデータの処理
    m_pICMPHandler->Process ();
    // 9. IGMPデータの処理
    m_pIGMPHandler->Process ();
}
```

**transportlayer**

```cpp
void CTransportLayer::Process (void) {
    // 1. バッファの作成
    u8 Buffer[FRAME_BUFFER_SIZE];
    // 2. ネットワークレイヤのRxQueueからデータを取得
    while (m_pNetworkLayer->Receive (Buffer, &nResultLength, &Sender, &Receiver, &nProtocol)) {
        for (i = 0; i < m_pConnection.GetCount (); i++) {
            // 3. Bufferをtcp/udpパケットして処理してRxQueueに入れる
            if (((CNetConnection *) m_pConnection[i])->PacketReceived (
                Buffer, nResultLength, Sender, Receiver, nProtocol) != 0) break;
        }
        if (i >= m_pConnection.GetCount ()) {
            // 4. 接続済のtcp/udpがなかった場合はRESETを送信
            m_TCPRejector.PacketReceived (Buffer, nResultLength,
                              Sender, Receiver, nProtocol);
        }
    }
    // 5. ネットワークレイヤで通知を受信
    while (m_pNetworkLayer->ReceiveNotification (&Type, &Sender, &Receiver,
                             &nSendPort, &nReceivePort, &nProtocol)) {
        for (i = 0; i < m_pConnection.GetCount (); i++) {
            // 5.1 受信した通知をTCP/DCPで処理
            if (((CNetConnection *) m_pConnection[i])->NotificationReceived (
                Type, Sender, Receiver, nSendPort, nReceivePort, nProtocol) != 0) break;
        }
    }

    // 6. 処理の継続/終了処理
    for (unsigned i = 0; i < m_pConnection.GetCount (); i++) {
        if (m_pConnection[i] != 0) {
            // 6.1 未完了であれば処理を継続
            if (!((CNetConnection *) m_pConnection[i])->IsTerminated ())
                ((CNetConnection *) m_pConnection[i])->Process ();
            // 6.2 kん量であれば接続を切る
            else {
                delete (CNetConnection *) m_pConnection[i];
                m_pConnection[i] = 0;
            }
        }
    }
    // 7. 完了したコネクションをリストから削除する
    m_SpinLock.Acquire ();
    // shrink m_pConnection
    unsigned nCount = m_pConnection.GetCount ();
    while (   nCount-- > 0 && m_pConnection[nCount] == 0) {
        m_pConnection.RemoveLast ();
    }
    m_SpinLock.Release ();
}

int CTransportLayer::Receive(void *pBuffer, int nFlags, int hConnection) {
    return ((CNetConnection *) m_pConnection[hConnection])->Receive (pBuffer, nFlags);
}
```

**tcpconnection**

```cpp
int CTCPConnection::Receive (void *pBuffer, int nFlags) {
    while ((nLength = m_RxQueue.Dequeue (pBuffer)) == 0) {
        switch (m_State) {
        case TCPStateSynSent:
        case TCPStateSynReceived:
        case TCPStateEstablished:
            break;
        }
    }
    return nLength;
}
```

### 送信処理

**transportlayer**

```cpp
int CTransportLayer::Send(const void *pData, unsigned nLength, int nFlags, int hConnection) {
    return ((CNetConnection *) m_pConnection[hConnection])->Send(pData, nLength, nFlags);
}

void CTransportLayer::Process (void) {
    for (unsigned i = 0; i < m_pConnection.GetCount (); i++) {
        if (m_pConnection[i] != 0) {
            if (!((CNetConnection *) m_pConnection[i])->IsTerminated ())
            {
                ((CNetConnection *) m_pConnection[i])->Process ();
            }
        }
    }
}
```

**tcpconnection**

```cpp
int CTCPConnection::Send(const void *pData, unsigned nLength, int nFlags) {
    switch (m_State) {
    case TCPStateSynSent:
    case TCPStateSynReceived:
    case TCPStateEstablished:
    case TCPStateCloseWait:
        break;
    }

    unsigned nResult = nLength;
    u8 *pBuffer = (u8 *) pData;

    while (nLength > FRAME_BUFFER_SIZE) {
        m_TxQueue.Enqueue (pBuffer, FRAME_BUFFER_SIZE);
        pBuffer += FRAME_BUFFER_SIZE;
        nLength -= FRAME_BUFFER_SIZE;
    }

    if (nLength > 0) {
        m_TxQueue.Enqueue(pBuffer, nLength);
    }
    return nResult;
}

void CTCPConnection::Process (void) {
    switch (m_State) {
    case TCPStateSynSent:
    case TCPStateSynReceived:
        if (m_bSendSYN) {
            m_bSendSYN = FALSE;
            if (m_State == TCPStateSynSent)
                SendSegment (TCP_FLAG_SYN, m_nISS);
            else
                SendSegment (TCP_FLAG_SYN | TCP_FLAG_ACK, m_nISS, m_nRCV_NXT);
            m_RTOCalculator.SegmentSent (m_nISS);
            StartTimer (TCPTimerRetransmission, m_RTOCalculator.GetRTO ());
        }
        return;
    }
}

boolean CTCPConnection::SendSegment (unsigned nFlags, u32 nSequenceNumber, u32 nAcknowledgmentNumber,
                     const void *pData, unsigned nDataLength) {
    return m_pNetworkLayer->Send (m_ForeignIP, TxBuffer, nPacketLength, IPPROTO_TCP);
}
```

**networklayer**

```cpp
boolean CNetworkLayer::Send (const CIPAddress &rReceiver, const void *pPacket, unsigned nLength,
                 int nProtocol, boolean bRouterAlert) {
    u8 PacketBuffer[nPacketLength];
    TIPHeader *pHeader = (TIPHeader *) PacketBuffer;
    pHeader->nVersionIHL          = IP_VERSION << 4 | nHeaderLength / 4;
    pHeader->nTypeOfService       = IP_TOS_ROUTINE;
    pHeader->nTotalLength         = le2be16 ((u16) nPacketLength);
    pHeader->nIdentification      = BE (IP_IDENTIFICATION_DEFAULT);
    pHeader->nFlagsFragmentOffset = IP_FLAGS_DF | BE (IP_FRAGMENT_OFFSET_FIRST);
    pHeader->nTTL                 = rReceiver.IsMulticast () ? IP_TTL_MULTICAST : IP_TTL_DEFAULT;
    pHeader->nProtocol            = (u8) nProtocol;
    memcpy (PacketBuffer+nHeaderLength, pPacket, nLength);

    return m_pLinkLayer->Send (*pNextHop, PacketBuffer, nPacketLength);
}
```

**linklayer**

```cpp
boolean CLinkLayer::Send (const CIPAddress &rReceiver, const void *pIPPacket, unsigned nLength) {
    u8 FrameBuffer[nFrameLength];
    TEthernetHeader *pHeader = (TEthernetHeader *) FrameBuffer;
    pHeader->nProtocolType = BE (ETH_PROT_IP);
    memcpy (FrameBuffer+sizeof (TEthernetHeader), pIPPacket, nLength);
    m_pNetDevLayer->Send (FrameBuffer, nFrameLength);
    return TRUE;
}
```

**netdevlayer**

```cpp
void CNetDeviceLayer::Send (const void *pBuffer, unsigned nLength) {
    m_TxQueue.Enqueue (pBuffer, nLength);
}

void CNetDeviceLayer::Process (void) {
    DMA_BUFFER (u8, Buffer, FRAME_BUFFER_SIZE);
    unsigned nLength;
    while (   m_pDevice->IsSendFrameAdvisable ()
           && (nLength = m_TxQueue.Dequeue (Buffer)) > 0) {
        m_pDevice->SendFrame (Buffer, nLength);
    }
}
```

**lan7800**

```cpp
boolean CLAN7800Device::SendFrame (const void *pBuffer, unsigned nLength) {
    DMA_BUFFER (u8, TxBuffer, FRAME_BUFFER_SIZE+TX_HEADER_SIZE);
    memcpy (TxBuffer+TX_HEADER_SIZE, pBuffer, nLength);
    u32 *pTxHeader = (u32 *) TxBuffer;
    pTxHeader[0] = (nLength & TX_CMD_A_LEN_MASK) | TX_CMD_A_FCS;
    pTxHeader[1] = 0;
    return GetHost ()->Transfer (m_pEndpointBulkOut, TxBuffer, nLength+TX_HEADER_SIZE) >= 0;
}
```
