# M5Cardputer LLM Assistant

M5CardputerとM5Module-LLMを使用したチャットアシスタントアプリケーションです。
キーボードから質問を入力すると、LLMが回答を生成し、テキストで応答します。

## 必要なハードウェア

- [M5Cardputer](https://docs.m5stack.com/en/core/Cardputer)
- [M5Module-LLM](https://docs.m5stack.com/en/module/M5Module-LLM)

## 開発環境

- [PlatformIO](https://platformio.org/)
- Arduino M5Stack Board Manager v2.0.7

## 依存ライブラリ

- [M5GFX](https://github.com/m5stack/M5GFX)
- [M5Unified](https://github.com/m5stack/M5Unified)
- [M5Module-LLM](https://github.com/m5stack/M5Module-LLM)

## セットアップ

1. PlatformIOをインストール
2. プロジェクトをクローン
```bash
git clone https://github.com/GOROman/LLMCardputer.git
```
3. 依存ライブラリをインストール
```bash
pio pkg install
```
4. ビルドとアップロード
```bash
pio run -t upload
```

## 使用方法

1. M5CardputerにM5Module-LLMを接続
2. 電源を入れると起動アニメーションが表示され、LLMの初期化が行われます
3. キーボードから質問を入力し、Enterキーで送信
4. LLMが回答を生成し、テキストで応答します

## 設定

`src/config.h` で以下の設定が可能です:

- フォント: `FONTNAME`
- シリアルポート: `MODULE_LLM_UART_RX`, `MODULE_LLM_UART_TX`
- LLMモデル: `MODEL`

## ライセンス

MIT License

## クレジット

- サウンド機能: [らびやん](https://gist.github.com/lovyan03/19e8a65195f85fbdd415558d149912f6)
