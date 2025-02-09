# LLMCardputer

***AI is in the palm of your hands***

<img width="432" alt="image" src="https://github.com/user-attachments/assets/8bf5a721-6b41-4780-80cb-878dc496f9ac" />


Large Language Models (LLMs) like OpenAI’s ChatGPT typically require a CUDA-capable GPU or an Apple Silicon Mac with substantial memory to run locally. However, in recent years, edge LLM modules designed for embedded microcontrollers have emerged.

In this article, we’ll explore how to combine M5Stack’s credit card-sized computer with a keyboard, Cardputer, and a ModuleLLM capable of running local LLMs. Together, they enable a "palm-sized device running a local LLM."

**↓ Demonstration**

https://x.com/GOROman/status/1883032143884103767


# How-To Guide

## What You’ll Need

### ModuleLLM

First, get the ModuleLLM (LLM module). It’s currently sold out at the official distributor, Switch Science, and M5Stack’s official store. I managed to snag the last one from Aliexpress. Production might resume after the Lunar New Year, so stock could return around March. Alternatively, you might find someone who bought it impulsively but doesn’t know how to use it and persuade them to sell it.

- [ModuleLLM](https://docs.m5stack.com/en/module/Module-LLM)
- [ModuleLLM - Switch Science](https://www.switch-science.com/products/10034)

### Cardputer

Next, get a Cardputer. Fortunately, its stock recently replenished.

- [Cardputer](https://docs.m5stack.com/en/core/Cardputer)
- [Cardputer - Switch Science](https://www.switch-science.com/products/9277)

## Step 1: Disassembly

Remove the ModuleLLM from its frame by unscrewing the four screws with a wrench. Disassemble the Cardputer as well. Be careful when detaching the large battery stuck with double-sided tape. Avoid bending it, as this could cause it to ignite. Remove the black component on top of the Cardputer too.

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/e9f33e0d-a8a6-6d01-96bc-a66c74095e12.png)

Modify the Cardputer’s case to fit the ModuleLLM snugly by trimming and sanding as needed.

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/d656d9cb-c5e5-ce5b-a606-b8dc422c1a6f.png)

Cut out a section at the back of the case to expose the M.BUS pins. To prevent short circuits, insulate the pins with Kapton or acetate tape.

#### Fitting Result

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/f667125d-5818-1972-7307-1683f092a009.png)

## Step 2: Wiring

You can communicate with ModuleLLM via UART or TCP (port 10001). For this guide, we’ll use UART.

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/27e74244-4d01-b4db-8f7e-0a9f5440c224.png)

Connect the ModuleLLM to the Cardputer as follows:

- **Cardputer GROVE terminal (G1, G2, +5V, GND)**
- **ModuleLLM UART pins (+5V, GND)**

While soldering directly to the board worked for this project, there may be better ways to connect them.

| Cardputer GROVE | ModuleLLM M.BUS |
|-----------------|-----------------|
| G (Black)       | GND             |
| 5V (Red)        | 5V              |
| G1 (Yellow)     | UART (Tx)       |
| G2 (White)      | UART (Rx)       |

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/b6b1f52c-d0f8-157c-7818-986509cff129.png)

## Step 3: Coding


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
