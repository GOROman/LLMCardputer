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

/** @brief エラーログの最大保持数 */
static const int MAX_ERROR_LOGS = 5;

/** @brief エラーログ */
static String error_logs[MAX_ERROR_LOGS];
/** @brief エラーログのインデックス */
static int error_log_index = 0;
/** @brief エラーログの表示フラグ */
static bool show_error_log = false;

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
 * @brief エラーログにメッセージを追加
 * @param msg エラーメッセージ
 * @param retry リトライ処理を行うかどうか
 * @return リトライ処理の結果 (リトライしない場合は常にtrue)
 */
static bool error_message(String msg, bool retry = false)
{
  // エラーログに追加
  error_logs[error_log_index] = "[" + String(millis()/1000) + "s] " + msg;
  error_log_index = (error_log_index + 1) % MAX_ERROR_LOGS;

  // エラー表示
  portENTER_CRITICAL_ISR(&display_mutex);
  canvas.setTextColor(WHITE, RED);
  canvas.println("Error: " + msg);
  if (retry) {
    canvas.println("Retrying...");
  }
  canvas.setTextColor(WHITE, BLACK);
  canvas.pushSprite(4, 4);
  portEXIT_CRITICAL_ISR(&display_mutex);

  M5Cardputer.Speaker.tone(440, 800); // Error
  delay(1000);

  // リトライ処理
  if (retry) {
    for (int i = 0; i < 3; i++) { // 最大3回リトライ
      if (module_llm.checkConnection()) {
        portENTER_CRITICAL_ISR(&display_mutex);
        canvas.setTextColor(GREEN);
        canvas.println("Retry successful");
        canvas.setTextColor(WHITE, BLACK);
        canvas.pushSprite(4, 4);
        portEXIT_CRITICAL_ISR(&display_mutex);
        return true;
      }
      delay(1000);
    }
    portENTER_CRITICAL_ISR(&display_mutex);
    canvas.setTextColor(WHITE, RED);
    canvas.println("Retry failed");
    canvas.setTextColor(WHITE, BLACK);
    canvas.pushSprite(4, 4);
    portEXIT_CRITICAL_ISR(&display_mutex);
    return false;
  }
  return true;
}

/**
 * @brief エラーログを表示
 */
static void show_error_logs()
{
  portENTER_CRITICAL_ISR(&display_mutex);
  canvas.setTextColor(WHITE, RED);
  canvas.println("=== Error Logs ===");
  for (int i = 0; i < MAX_ERROR_LOGS; i++) {
    if (error_logs[i].length() > 0) {
      canvas.println(error_logs[i]);
    }
  }
  canvas.println("================");
  canvas.setTextColor(WHITE, BLACK);
  canvas.pushSprite(4, 4);
  portEXIT_CRITICAL_ISR(&display_mutex);
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
    if (!error_message("Reset LLM failed", true)) {
      return;
    }
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
    llm_config.max_token_len = MAX_TOKEN_LENGTH;

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
      if (!error_message("Setup LLM failed", true)) {
        delay(500);
        continue;
      }
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
/**
 * @brief プログレスバーを表示
 * @param progress 進捗率(0-100)
 */
static void show_progress(int progress)
{
  const int BAR_WIDTH = M5Cardputer.Display.width() - 40;
  const int BAR_HEIGHT = 10;
  const int BAR_X = 20;
  const int BAR_Y = M5Cardputer.Display.height() - 20;

  portENTER_CRITICAL_ISR(&display_mutex);
  M5Cardputer.Display.fillRect(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, BLACK);
  M5Cardputer.Display.drawRect(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, WHITE);
  if (progress > 0) {
    int width = (BAR_WIDTH - 2) * progress / 100;
    M5Cardputer.Display.fillRect(BAR_X + 1, BAR_Y + 1, width, BAR_HEIGHT - 2, CYAN);
  }
  portEXIT_CRITICAL_ISR(&display_mutex);
}

void talk(String question)
{
  unsigned long start_time = millis();
  bool timeout = false;
  int progress = 0;

  // Push question to LLM module and wait inference result
  module_llm.llm.inferenceAndWaitResult(
    llm_work_id, 
    question.c_str(), 
    [&start_time, &timeout, &progress](String &result) {
      if (millis() - start_time > LLM_TIMEOUT) {
        timeout = true;
        return;
      }
      answer += result;
      progress = min(95, progress + 5);  // 最大95%まで
      show_progress(progress);
    }, 
    2000, 
    "llm_inference"
  );

  if (timeout) {
    error_message("LLM response timeout", false);
    show_progress(0);  // プログレスバーをクリア
    return;
  }

  show_progress(100);  // 完了
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
 
  // 起動アニメーション
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

      // 特殊キーの処理
      if (status.fn)
      {
        // Fn + L: エラーログ表示
        bool hasL = false;
        for (auto c : status.word) {
          if (c == 'L') {
            hasL = true;
            break;
          }
        }
        if (hasL) {
          show_error_logs();
          return;
        }

        // Fn + C: 画面クリア
        bool hasC = false;
        for (auto c : status.word) {
          if (c == 'C') {
            hasC = true;
            break;
          }
        }
        if (hasC) {
          clear();
          return;
        }
      }
      else
      {
        // 通常の文字入力
        for (auto i : status.word)
        {
          // 入力バッファのサイズ制限
          if (data.length() >= MAX_INPUT_LENGTH)
          {
            error_message("Input buffer full", false);
            return;
          }
          data += i;
        }
      }

      // バックスペース
      if (status.del && data.length() > 1)  // '>' は削除しない
      {
        data.remove(data.length() - 1);
      }

      // エンターキー
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
