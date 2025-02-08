/**
 * @file main.cpp
 * @brief M5Cardputer LLM Assistant
 * @version 0.1
 * @date 2024-02-08
 *
 * @details
 * M5CardputerとM5Module-LLMを使用したチャットアシスタントアプリケーション。
 * キーボード入力による対話、音声フィードバック、マルチタスクによる非同期処理を実装。
 *
 * @Hardwares: M5Cardputer with M5Module-LLM
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5Module-LLM: https://github.com/m5stack/M5Module-LLM
 */

#include <M5GFX.h>
#include <M5Cardputer.h>
#include <M5ModuleLLM.h>
#include "config.h"
#include "sound.h"

/** @brief LLMモジュールのインスタンス */
static M5ModuleLLM module_llm;

/** @brief LLMのワークID */
static String llm_work_id;
/** @brief ユーザーからの質問文 */
static String question;
/** @brief LLMからの回答文 */
static String answer;
/** @brief 入力バッファ */
static String data;

/** @brief LLMの初期化完了フラグ */
static bool task_llm_ready = false;
/** @brief LLMの回答完了フラグ */
static bool end_flag = false;

/** @brief 描画用キャンバス */
static M5Canvas canvas(&M5Cardputer.Display);

/** @brief ディスプレイ描画用の排他制御ミューテックス */
static portMUX_TYPE display_mutex = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief ビープ音を鳴らす
 * @details 440Hz(A4)の矩形波を50ms再生
 */
static void beep()
{
  sound_play(SOUND_SQUARE, 440, 50);
}

/**
 * @brief エラーメッセージを表示
 * @param msg 表示するメッセージ
 * @details 赤背景でメッセージを表示し、エラー音を鳴らす
 */
static void error_message(String msg)
{
  portENTER_CRITICAL_ISR(&display_mutex);
  canvas.setTextColor(WHITE, RED);
  canvas.println(msg);
  canvas.setTextColor(WHITE, BLACK);
  canvas.pushSprite(4, 4);
  portEXIT_CRITICAL_ISR(&display_mutex);

  M5Cardputer.Speaker.tone(440, 800); // Error
  delay(1000);
}

/**
 * @brief システムメッセージを表示
 * @param msg 表示するメッセージ
 * @param lf 改行の有無
 * @details シアン色でメッセージを表示
 */
static void system_message(String msg, bool lf = true)
{
  portENTER_CRITICAL_ISR(&display_mutex);
  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                               M5Cardputer.Display.width(), 28,
                               BLACK);

  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.drawString(msg, 4,
                                 M5Cardputer.Display.height() - 24);

  portEXIT_CRITICAL_ISR(&display_mutex);
}

/**
 * @brief LLMの初期化
 * @param lang 使用する言語 ("en":英語, "jp":日本語)
 * @details LLMのリセット、設定、プロンプト設定を行う
 */
static void LLM_setup(String lang)
{
  // Reset LLM
  int err = module_llm.sys.reset();
  if (err != MODULE_LLM_OK)
  {
    error_message("Error: Reset LLM failed");
    return;
  }

  // Setup LLM
  while (1)
  {
    system_message("Setup LLM");

    m5_module_llm::ApiLlmSetupConfig_t llm_config;
    if (MODEL)
    {
      llm_config.model = MODEL;
    }
    llm_config.max_token_len = 1023;

    if (lang == "en")
    {
      llm_config.prompt = "Please answer in English";
    }
    else if (lang == "jp")
    {
      llm_config.prompt = "Please answer in Japanese.";
    }

    llm_work_id = module_llm.llm.setup(llm_config);
    if (llm_work_id == "")
    {
      error_message("Error: Setup LLM failed");
      delay(500);
      continue;
    }
    system_message("LLM Work ID:" + llm_work_id);
    break;
  }

  beep();
}

/**
 * @brief LLM制御タスク
 * @param pvParameters タスクパラメータ(未使用)
 * @details LLMモジュールの初期化と定期的な状態更新を行う
 */
void task_llm(void *pvParameters)
{
  // Init module
  module_llm.begin(&Serial2);

  // Make sure module is connected
  system_message("ModuleLLM connecting", false);

  while (1)
  {
    if (module_llm.checkConnection())
    {
      break;
    }
  }

  LLM_setup("jp");
  task_llm_ready = true;

  while (1)
  {
    module_llm.update();
    vTaskDelay(100);
  }
}

/**
 * @brief LLMに質問を送信
 * @param question 質問文
 * @details LLMに質問を送信し、非同期で回答を受け取る
 */
void talk(String question)
{
  // Push question to LLM module and wait inference result
  module_llm.llm.inferenceAndWaitResult(llm_work_id, question.c_str(), [](String &result)
                                        { answer += result; }, 2000, "llm_inference");
  end_flag = true;
}

/**
 * @brief 回答表示タスク
 * @param pvParameters タスクパラメータ(未使用)
 * @details LLMからの回答を1文字ずつ表示し、音声フィードバックを行う
 */
void task_print(void *pvParameters)
{
  while (1)
  {
    int len = answer.length();
    String buffer = answer;
    answer = "";
    int count = 0;

    for (int i = 0; i < len; i++)
    {
      // 1文字づつ表示
      String str = buffer.substring(i, i + 1);

      if (str == " " || str == "?")
      {
      }
      else
      {
        count++;
        if (count % 2 == 1)
        {
          // 喋ってる風のSE
          sound_play(SOUND_SQUARE, 888, 33); // 888Hz(A5)
          vTaskDelay(33);
        }
      }

      portENTER_CRITICAL_ISR(&display_mutex);
      canvas.setTextColor(GREEN);
      canvas.printf("%s", str.c_str());
      canvas.pushSprite(4, 4);
      portEXIT_CRITICAL_ISR(&display_mutex);
    }

    if (end_flag && (len == 0))
    {
      sound_play_SE(SOUND_SE_END);
      end_flag = false;
    }

    vTaskDelay(50);
  }
}

/**
 * @brief 画面をクリア
 * @details キャンバスをクリアして再描画
 */
void clear()
{
  portENTER_CRITICAL_ISR(&display_mutex);
  canvas.setCursor(0, 0);
  canvas.clear();
  canvas.pushSprite(4, 4);
  portEXIT_CRITICAL_ISR(&display_mutex);
}

/**
 * @brief 起動アニメーションを表示
 * @details ランダムなパターンと音を使用した起動時のアニメーション効果を表示
 */
static void startup_animation()
{
  const int W = M5Cardputer.Display.width();
  const int H = M5Cardputer.Display.height();
  const int STEP = 20;
  const uint16_t COLOR[] = {
      BLACK,
      M5Cardputer.Display.color565(175, 66, 47),
      M5Cardputer.Display.color565(139, 227, 77),
      M5Cardputer.Display.color565(19, 17, 169),
  };
  while (!task_llm_ready)
  {
    portENTER_CRITICAL_ISR(&display_mutex);
    M5Cardputer.Display.startWrite();
    for (int x = 0; x < W; x += STEP)
    {
      for (int y = 0; y < H; y += STEP)
      {
        int r = rand() % 4;

        uint16_t color = COLOR[r];
        canvas.fillRect(x, y, STEP, STEP, color);
      }
    }
    canvas.pushSprite(4, 4);
    M5Cardputer.Display.endWrite();
    portEXIT_CRITICAL_ISR(&display_mutex);

    // ランダムに S&H 風の音を鳴らす
    int freq = 400 + rand() % 800;
    sound_play(SOUND_TRIANGLE, (float)freq, 80);

    vTaskDelay(100);
  }
  clear();
}

/**
 * @brief セットアップ
 * @details デバイスの初期化、タスクの作成、起動アニメーションの表示を行う
 */
void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  // Module LLMのUART
  Serial2.begin(115200, SERIAL_8N1, MODULE_LLM_UART_RX, MODULE_LLM_UART_TX);

  M5Cardputer.Speaker.setVolume(SOUND_VOLUME);

  portENTER_CRITICAL_ISR(&display_mutex);
  M5Cardputer.Display.startWrite();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1.0f);

  M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(),
                               M5Cardputer.Display.height() - 28, WHITE);
  M5Cardputer.Display.drawRect(1, 1, M5Cardputer.Display.width() - 1,
                               M5Cardputer.Display.height() - 29, WHITE);
  M5Cardputer.Display.setFont(&FONTNAME);

  canvas.setFont(&FONTNAME);
  canvas.setTextSize(1.0);
  canvas.createSprite(M5Cardputer.Display.width() - 8,
                      M5Cardputer.Display.height() - 36);
  canvas.setTextScroll(true);
  canvas.pushSprite(4, 4);
  M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);
  M5Cardputer.Display.endWrite();
  portEXIT_CRITICAL_ISR(&display_mutex);

  xTaskCreate(
      task_llm, "task_llm", 4096, NULL, 1, NULL);

  clear();

  xTaskCreate(
      task_print, "task_print", 4096, NULL, 1, NULL);
 
  startup_animation();

  canvas.setTextColor(GREEN);
  canvas.pushSprite(4, 4);

  data = ">";
  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                               M5Cardputer.Display.width(), 28,
                               BLACK);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.drawString(data, 4,
                                 M5Cardputer.Display.height() - 24);

  // 初期の指示
  talk("Please introduce yourself.");
}

/**
 * @brief メインループ
 * @details キーボード入力の処理、テキスト表示の更新を行う
 */
void loop()
{
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      for (auto i : status.word)
      {
        M5.Speaker.tone(880, 100);
        data += i;
      }

      if (status.del)
      {
        M5.Speaker.tone(440, 100);
        data.remove(data.length() - 1);
      }

      if (status.enter)
      {
        sound_play_SE(SOUND_SE_START);
        data.remove(0, 1);
        canvas.setTextColor(WHITE);

        M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                                     M5Cardputer.Display.width(), 28,
                                     BLACK);
        {
          canvas.println("\n[You]:" + data);
          canvas.pushSprite(4, 4);

          canvas.setTextColor(GREEN);
          canvas.print("[AI]:");
          question = data;
          data = "";

          talk(question);
        }

        data = ">";
      }

      M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                                   M5Cardputer.Display.width(), 28,
                                   BLACK);

      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.drawString(data, 4,
                                     M5Cardputer.Display.height() - 24);
    }
  }
}
