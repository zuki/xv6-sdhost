# [rpi-os](https://github.com/Hongqin-Li/rpi-os)にWiFi機能を追加する

- [rpi-os](https://github.com/zuki/rpi-os)はSDカードとの接続にmmcを使用している。
- このプロジェクトではsdカードとの接続にsdhostを使用し、mmcをwifiとの接続に使用する.

# 開発履歴

- (2025/08/19) `USE_SDHOST`を定義し、一部修正でsdhostでの稼働を確認