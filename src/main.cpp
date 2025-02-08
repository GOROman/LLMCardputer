/**
 * @file main.cpp
 * @brief M5Cardputer LLM Assistant
 * @version 0.1
 * @date 2024-02-08
 *
 * @Hardwares: M5Cardputer with M5Module-LLM
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5Module-LLM: https://github.com/m5stack/M5Module-LLM
 */

#include "M5Cardputer.h"
#include "M5GFX.h"
#include <M5ModuleLLM.h>

M5Canvas canvas(&M5Cardputer.Display);
M5ModuleLLM module_llm;
String llm_work_id;
String data = "> ";

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(0.5);
    M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(),
                                M5Cardputer.Display.height() - 28, GREEN);
    M5Cardputer.Display.setFont(&fonts::FreeSerifBoldItalic18pt7b);

    M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 4,
                                M5Cardputer.Display.width(), 4, GREEN);

    canvas.setFont(&fonts::FreeSerifBoldItalic18pt7b);
    canvas.setTextSize(0.5);
    canvas.createSprite(M5Cardputer.Display.width() - 8,
                       M5Cardputer.Display.height() - 36);
    canvas.setTextScroll(true);

    // Init module serial port for M5Cardputer
    Serial2.begin(115200, SERIAL_8N1, 13, 14);

    // Init LLM module
    module_llm.begin(&Serial2);

    canvas.println(">> Check ModuleLLM connection..");
    canvas.pushSprite(4, 4);
    while (1) {
        if (module_llm.checkConnection()) {
            break;
        }
        delay(100);
    }

    canvas.println(">> Reset ModuleLLM..");
    canvas.pushSprite(4, 4);
    module_llm.sys.reset();

    canvas.println(">> Setup LLM..");
    canvas.pushSprite(4, 4);
    m5_module_llm::ApiLlmSetupConfig_t llm_config;
    llm_config.max_token_len = 1023;
    llm_work_id = module_llm.llm.setup(llm_config);

    canvas.println(">> Ready! Type your question and press Enter");
    canvas.pushSprite(4, 4);
    M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);
}

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto i : status.word) {
                data += i;
            }

            if (status.del) {
                data.remove(data.length() - 1);
            }

            if (status.enter) {
                String question = data.substring(2); // Remove "> "
                canvas.setTextColor(TFT_GREEN);
                canvas.println(question);
                canvas.setTextColor(TFT_YELLOW);
                canvas.printf(">> ");
                canvas.pushSprite(4, 4);

                // Send question to LLM module and wait for response
                module_llm.llm.inferenceAndWaitResult(llm_work_id, question.c_str(), 
                    [](String& result) {
                        canvas.printf("%s", result.c_str());
                        canvas.pushSprite(4, 4);
                    }
                );

                canvas.println();
                canvas.pushSprite(4, 4);
                data = "> ";
            }

            M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                                       M5Cardputer.Display.width(), 25,
                                       BLACK);

            M5Cardputer.Display.drawString(data, 4,
                                         M5Cardputer.Display.height() - 24);
        }
    }
    delay(20);
}
