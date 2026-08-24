# wifiの実装 : Circleの`addon/wlan`を移植

## Raspberry Pi 3B＋のWiFi機能

- WiFiチップ CYW4355が搭載されておりこれがwifi通信を行う。
- カーネルはこのチップとSDIO経由で通信する。
- RasPi3B+にはSDカードインタフェースとしてARASAN EMMCと独自SDHostの2種類が提供されており、
  デフォルトではARASAN EMMCがSDカードとの通信に利用されている。
- ARASAN EMMCはSDIO™カード規格 Ver.3.0にも対応している。
- Wifi機能の追加にあたって、SDカードをSDHost経由とし、ARASAN EMMCをSDIO用とした。
    - pin[34-39]をALT3で初期化しARASAN EMMCをSDIOとしてWiFiチップとの通信に使用する
    - pin[48-53]をALT0で初期化しSDHostとしてSDカードとの通信に使用する
- CircleのwlanはPlan9のコードをC++に変換して使用しているが、Plan9との接続用CircleコードをCに変換して使用した。
    - [ether4330.c](https://9p.io/sources/contrib/miller/9/bcm/ether4330.c)
- wpa_supplicant（wifiルータへの接続処理）にはaddon/wlanにも使われているhostapを使用し、
  circle独自関数をxv6に合わせて修正した。

## Plan9コードについての覚書

- `struct Rendez`はspinlockのchanとして使用する一意なアドレスとして使用する
- `waserror/error`による例外処理機構で一連の処理におけるエラーをまとめて扱う。そのまま使用した。

## google aiの活用によるコード修正

- デバッグ出力だけでは発見できないエラー、ストールについては[google ai](https://google.com/ai)が非常に役立った。
  行き詰まった際に何らかの突破口が提示されるのがありがたかった。
- 漠然とした質問には既に知っていることしか回答がないので、問題点をコートやログ出力で
  ピンポイントで示す必要があった。
- 明らかな間違いは指摘するとすぐ訂正があった。
- 特に以下の点が有益だった。
    - kthread_create()関数の修正提案。
    - ストール、データの混在等を多数発生させていたlock操作の不備の特定と修正提案。
    - wlanコードとtcp/ipプロトコルスタックとの連携部分のコード提案

## 開発時の主な問題点

1. wpasupplicant_init()でwpa_supplicant.confが開けない
    - fatfsのパス名は`/d/file`で指定するが、内部では`4:/file`として使用。

2. net_init(), net_run(), wpa_supplicant_init()の実行順所
    - 有線LANを接続しなとnet_init()でストール
    - wpa_supplicant_init()でエラー

3. `sdhost.c#L1330`の`assert(host->mrq == 0);`がアサーションエラー
    - このエラーの解決に一番時間がかかった。`struct bcm4343 *host`のロック不全が原因だった。
    - host->busyフラグの導入によるhostのシリアル化で解決

    ```c
    void sdhost_request_sync(struct bcm2835_host *host, struct mmc_request *req)
    {
        req->done = 0;

        while (host->busy) {                    // hostが使用中の場合はsleep
            host->sleep_fn(host->sleep_arg);
        }

        host->busy = 1;                         // hostの使用権を獲得してrequest
        sdhost_request(host, &host->mmc, req);

        while (!req->done) {                    // requestの終了を待ってsleep
            host->sleep_fn(host->sleep_arg);
        }
        host->mrq = 0;
        host->busy = 0;
        wakeup(host->sleep_arg);
    }
    ```

4. kmalloc()の上限(4080)を超える要求
    - コードを修正することで上限を廃止できた

5. string関数の間違いが思いがけないエラーに繋がっていることが数多く判明した。
    - すべてmuslのコードに置き換えた

6. カーネルスタックのオーバーフローが発生
    - カーネルスレッド作成時にスタックページ数を指定可能にした

7. 数カ所でNULLポインタをkmfreeしていた

8. bcm4343_recv_scan_result()のqueue_pop()でNULL判定が効かない
    - NULL判定にqueue.num==0を追加で回避した
    - しかし、bcm4343_scan_result_recv()のqueue_popではNULL判定が効き、
      逆にqueue.num==0を追加したことでNULL判定されて処理されないという気付きにくい
      問題が発生した

9. l2_packetのethertypeによる切り分けの問題
    - 当初、l2_packet_receive()で行おうとしたがうまく行かず、最上流の
      bcm4343_net_handler()で行うように変更した

10. Plan9の例外処理機構の理解不足でプロセス間でエラーが混在
    - errorstackの持ち方を修正

11. wpa_supplicantの実行で、shellが立ち上がらない
    - bufcache.cのget_block()のヒット判定条件に誤り（`cur->flags & BCF_ALLOCATED`を追加）

12. struct bcm4343のnet_deviceとしての初期化で未設定のフィールドが存在することで各所で問題発生
    - `ifconfig net3`でetherアドレスが表示されない、ip_output_device()でARPのリゾルブが開始されないなど
    - ether_setup_helper()で設定しているフィールドすべてbcm4343_init()でセット

13. `bin/init`のget_file()でストール
    - eloop_run()でos_sleep()が正常に機能していないことが発覚し、この関数の実装に問題があることが判明
    - delayus()ではなくjiffiesによるsleepにロジックを変更

14. デバッグ出力が交じる問題
    - ロックを`struct serial_channel g_channel.lock`に統一
    - cprintf()（この関数を使うwpa_printfなどを含む）によるログ出力が混在しなくなる

15. /bin/dns (その後 /bin/ntpに改名)がストール
    - EAPOLパケットが受信できずEAPがdisableとなり認証できない
    - 9項に8項が絡む問題だった。queue_popではNULL判定でqueue.num==0を削除したら受信できるようになった

16. ethertypeによる振り分けをbcm4343_net_handler()で行うよう変更

17. arpリゾルブができない
    - arpリゾルブ要求を行い応答も来ているが、応答が来る前に応答なし判定しているためだった
    - arpリゾルブをsleepして待つこととし、arpキャッシュ挿入、更新時にwakeupするようなロジックを追加

18. wlanによるntp時刻取得ができた

    ```bash
    $ /bin/date
    1900年 1日 1日 月曜日  9時 0分15秒 JST
    $ /bin/ntp
    $ /bin/date
    2026年 8日23日 日曜日 17時59分22秒 JST              // ntpから設定できている
    ```
$
```

## wlanのデバッグレベルの設定

1. `hostap/src/util/wpa_debug_xv6.c`の`wpa_debug_level`
2. `hostap/wpa_wupplicat/main_xv6.c`の`wpa_supplicant_main()`の`params.wpa_debug_level`

## ether4343のデバッグの設定

1. `ether4343.c`のSDIODEBUG, SBDEBUG, EVENTDEBUG, VARDEBUG, FWDEBUG
2. `ether4343.c`の`static int iodebug`
