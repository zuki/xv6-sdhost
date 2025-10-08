#ifndef INC_NET_IF_H
#define INC_NET_IF_H

/*    $NetBSD: if.h,v 1.282 2020/02/14 22:04:12 thorpej Exp $    */

/*-
 * Copyright (c) 1999, 2000, 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by William Studenmund and Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    @(#)if.h    8.3 (Berkeley) 2/9/95
 */
// デバイスフラグ (IOCGIFFLAGS, SIOCSIFFLAGSで取得・設定する)
#define    IFF_UP           0x0001        /* インターフェースは動作中 */
#define    IFF_BROADCAST    0x0002        /* 有効なブロードキャストアドレスがセットされている */
#define    IFF_DEBUG        0x0004        /* 内部のデバッグフラグ */
#define    IFF_LOOPBACK     0x0008        /* インターフェースはループバックである */
#define    IFF_POINTOPOINT  0x0010        /* インターフェースは point-to-point リンクである */
/*            0x0020           was IFF_NOTRAILERS */
#define    IFF_RUNNING      0x0040        /* リソースが割り当て済み */
#define    IFF_NOARP        0x0080        /* arp プロトコルがない。 L2宛先アドレスが設定されていない*/
#define    IFF_PROMISC      0x0100        /* インターフェースは promiscuous モードである */
#define    IFF_ALLMULTI     0x0200        /* すべてのマルチキャストパケットを受信する */
#define    IFF_MASTER       0x0400        /* 負荷分散グループのマスターである */
#define    IFF_SLAVE        0x0800        /* 負荷分散グループのスレーブである */
#define    IFF_MULTICAST    0x1000        /* マルチキャストをサポートしている */
#define    IFF_PORTSEL      0x2000        /* ifmap によってメディアタイプを選択できる */
#define    IFF_AUTOMEDIA    0x4000        /* 動メディア選択が有効になっている */
#define    IFF_DYNAMIC      0x8000        /* このインターフェースが閉じるとアドレスは失われる */
#define    IFF_LOWER_UP     0x10000       /* ドライバからの L1 アップの通知 */
#define    IFF_DORMANT      0x20000       /* ドライバからの休止状態の通知 */
#define    IFF_ECHO         0x40000       /* 送られたパケットをエコーする*/
#endif
