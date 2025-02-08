#pragma once

// Powered by らびやん
// https://gist.github.com/lovyan03/19e8a65195f85fbdd415558d149912f6

#include <M5Unified.h>

// Wave type
enum
{
    SOUND_SQUARE = 0,
    SOUND_TRIANGLE = 1,
};

// SE type
enum SOUND_SE
{
    SOUND_SE_START = 0,
    SOUND_SE_END = 1,
    SOUND_SE_TALK = 2,
};

#define BPM 200
#define NOTE_32_MS (60000 / BPM / 8)
#define NOTE_64_MS (60000 / BPM / 16) 

typedef struct
{
    int freq;     // 周波数 (0は休符)
    int duration; // ms
} Note;


// 音を鳴らす(波形種類・周波数・時間)
void sound_play(const int type, const float freq, const uint32_t duration);

// SE再生
void sound_play_SE(const SOUND_SE no);
