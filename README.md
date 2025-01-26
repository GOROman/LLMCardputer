# LLMCardputer

***AI is in the palm of your hands***

Large Language Models (LLMs) like OpenAI’s ChatGPT typically require a CUDA-capable GPU or an Apple Silicon Mac with substantial memory to run locally. However, in recent years, edge LLM modules designed for embedded microcontrollers have emerged.

In this article, we’ll explore how to combine M5Stack’s credit card-sized computer with a keyboard, Cardputer, and a ModuleLLM capable of running local LLMs. Together, they enable a "palm-sized device running a local LLM."

**↓ Demonstration**

https://x.com/GOROman/status/1883032143884103767


# How-To Guide

## What You’ll Need

### ModuleLLM

First, get the ModuleLLM (LLM module). It’s currently sold out at the official distributor, Switch Science, and M5Stack’s official store. I managed to snag the last one from Aliexpress. Production might resume after the Lunar New Year, so stock could return around March. Alternatively, you might find someone who bought it impulsively but doesn’t know how to use it and persuade them to sell it.

- [ModuleLLM - Switch Science](https://www.switch-science.com/products/10034)

### Cardputer

Next, get a Cardputer. Fortunately, its stock recently replenished.

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
| G2 (Yellow)     | UART (Tx)       |
| G1 (White)      | UART (Rx)       |

![image.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/30621/b6b1f52c-d0f8-157c-7818-986509cff129.png)

## Step 3: Coding

### Write the Code

Start by modifying the official sample code to fit your needs.

- [SerialTextAssistant.ino (ModuleLLM Example)](https://github.com/m5stack/M5Module-LLM/blob/main/examples/SerialTextAssistant/SerialTextAssistant.ino)

- [inputText.ino (Cardputer Example)](https://github.com/m5stack/M5Cardputer/blob/master/examples/Basic/keyboard/inputText/inputText.ino)

We combined these examples and added functionality to play sounds when keys are pressed.

Define the GPIO pins for UART (TX, RX) as follows:
```c
Serial2.begin(115200, SERIAL_8N1, 1, 2);
