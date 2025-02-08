/**
 * @file sound.h
 * @brief M5Cardputer用サウンド機能のヘッダーファイル
 * @version 0.1
 * @date 2024-02-08
 * 
 * @details
 * M5Cardputerのスピーカーを使用して音を再生するための機能を提供します。
 * 矩形波と三角波の2種類の波形に対応し、効果音(SE)の再生機能も備えています。
 * 
 * @note
 * Original code by らびやん
 * @see https://gist.github.com/lovyan03/19e8a65195f85fbdd415558d149912f6
 */

#pragma once
#include <M5Unified.h>

/**
 * @brief 波形の種類を定義する列挙型
 */
enum
{
    SOUND_SQUARE = 0,    ///< 矩形波 (Duty比50%)
    SOUND_TRIANGLE = 1,  ///< 三角波
};

/**
 * @brief 効果音(SE)の種類を定義する列挙型
 */
enum SOUND_SE
{
    SOUND_SE_START = 0,  ///< 起動時のSE
    SOUND_SE_END = 1,    ///< 終了時のSE
    SOUND_SE_TALK = 2,   ///< 会話時のSE
};

// 音楽の速さを定義
#define BPM 200                   ///< テンポ(1分間の拍数)
#define NOTE_32_MS (60000 / BPM / 8)  ///< 32分音符の長さ(ms)
#define NOTE_64_MS (60000 / BPM / 16) ///< 64分音符の長さ(ms)

/**
 * @brief 音符を表す構造体
 */
typedef struct
{
    int freq;     ///< 周波数 (Hz) (0は休符)
    int duration; ///< 音の長さ (ms)
} Note;


/**
 * @brief 指定した波形、周波数、時間で音を再生する
 * 
 * @param type 波形の種類 (SOUND_SQUARE または SOUND_TRIANGLE)
 * @param freq 周波数 (Hz)
 * @param duration 再生時間 (ms)
 */
void sound_play(const int type, const float freq, const uint32_t duration);

/**
 * @brief 指定した効果音(SE)を再生する
 * 
 * @param no 効果音の種類 (SOUND_SE_START, SOUND_SE_END, SOUND_SE_TALK)
 */
void sound_play_SE(const SOUND_SE no);
