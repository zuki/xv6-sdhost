# [rpi-os](https://github.com/Hongqin-Li/rpi-os)にWiFi機能を追加する

- [rpi-os](https://github.com/zuki/rpi-os)はSDカードとの接続にmmcを使用している。
- このプロジェクトではsdカードとの接続にsdhostを使用し、mmcをwifiとの接続に使用する.

# 開発履歴

- (2025/08/19) `USE_SDHOST`を定義し、一部修正でsdhostでの稼働を確認 (commit: bc4039b)
- (2025/08/20) buddyとslab, kmallocを追加 (commit: 24a6584, 9ae687f, c4ba874)
- (2025/08/20) ブロックサイズを4096に拡大 (commit: f8be6d4)
- (2025/08/22) ログシステムを廃棄 (commit: c4ba874)
- (2025/09/09) 時計機能を追加 (commit: 8fade0c, 9bdb4b5)
- (2025/09/15) USB機能を追加 (commit: c494065)
- (2025/10/11) ネットワーク機能を追加 (ad0fff1)
