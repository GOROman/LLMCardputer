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

### フォント設定
- `FONTNAME`: 日本語表示に対応したLGFXJapanGothicフォントを使用(デフォルト: fonts::lgfxJapanGothic_24)

### 通信設定
- `MODULE_LLM_UART_RX`: UART受信ピン(デフォルト: 2)
- `MODULE_LLM_UART_TX`: UART送信ピン(デフォルト: 1)
- M5Cardputerの Serial2 を使用してM5Module-LLMと通信

### サウンド設定
- `SOUND_VOLUME`: スピーカーの音量(0-255の範囲、0: 無音, 255: 最大音量)
  デフォルト: 100

### LLMモデル設定
- `MODEL`: 使用するLLMモデルを指定(デフォルト: NULL)
  - NULL: デフォルトモデルを使用
  - 利用可能なモデル:
    - qwen2.5-0.5B-prefill-20e: 小規模な高速モデル
    - qwen2.5-1.5b-ax630c: 中規模な汎用モデル
    - deepseek-r1-1.5B-ax630c: DeepSeekベースの高性能モデル

## ライセンス

MIT License

## クレジット

- サウンド機能: [らびやん](https://gist.github.com/lovyan03/19e8a65195f85fbdd415558d149912f6)