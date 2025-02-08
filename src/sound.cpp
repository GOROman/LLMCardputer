// Powered by らびやん
// https://gist.github.com/lovyan03/19e8a65195f85fbdd415558d149912f6

#include "sound.h"

// Square Wave: Duty 50%
static constexpr const uint8_t step_square_wave[] = {
    0,
    0,
    0,
    0,
    127,
    127,
    127,
    127,
};

// Triangle Wave
static constexpr const uint8_t step_triangle_wave[] = {
    136,
    136,
    136,
    136,
    153,
    153,
    153,
    153,
    170,
    170,
    170,
    170,
    187,
    187,
    187,
    187,
    204,
    204,
    204,
    204,
    221,
    221,
    221,
    221,
    238,
    238,
    238,
    238,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    238,
    238,
    238,
    238,
    221,
    221,
    221,
    221,
    204,
    204,
    204,
    204,
    187,
    187,
    187,
    187,
    170,
    170,
    170,
    170,
    153,
    153,
    153,
    153,
    136,
    136,
    136,
    136,
    119,
    119,
    119,
    119,
    102,
    102,
    102,
    102,
    85,
    85,
    85,
    85,
    68,
    68,
    68,
    68,
    51,
    51,
    51,
    51,
    34,
    34,
    34,
    34,
    17,
    17,
    17,
    17,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    17,
    17,
    17,
    17,
    34,
    34,
    34,
    34,
    51,
    51,
    51,
    51,
    68,
    68,
    68,
    68,
    85,
    85,
    85,
    85,
    102,
    102,
    102,
    102,
    119,
    119,
    119,
    119,
};

static Note MELODY_SE_START[] = {
    {261, NOTE_32_MS}, // C4 (261.63Hz)
    {0, NOTE_32_MS},   // 休符
    {392, NOTE_32_MS}, // G4 (392.00Hz)
    {0, NOTE_32_MS},   // 休符
    {294, NOTE_32_MS}, // D4 (293.66Hz)
    {0, NOTE_32_MS},   // 休符
    {440, NOTE_32_MS}, // A4 (440.00Hz)
    {-1, -1},         // Terminater
};

static Note MELODY_SE_END[] = {
    {349, NOTE_64_MS}, // F4
    {0, NOTE_64_MS},   // r
    {523, NOTE_64_MS}, // C5
    {0, NOTE_64_MS},   // r
    {329, NOTE_64_MS}, // E4
    {0, NOTE_64_MS},   // r
    {494, NOTE_64_MS}, // B4
    {0, NOTE_64_MS},   // r
    {622, NOTE_64_MS}, // D#4
    {0, NOTE_64_MS},   // r
    {440, NOTE_64_MS}, // A4
    {0, NOTE_64_MS},   // r
    {294, NOTE_64_MS}, // D4
    {0, NOTE_64_MS},   // r
    {440, NOTE_64_MS}, // A4
    {0, NOTE_64_MS},   // r
    {554, NOTE_64_MS}, // C#4
    {0, NOTE_64_MS},   // r
    {415, NOTE_64_MS}, // G#4
    {0, NOTE_64_MS},   // r
    {261, NOTE_32_MS}, // C4
    {0, NOTE_32_MS},   // r
    {523, NOTE_32_MS}, // C5
    {0, NOTE_32_MS},   // r
    {1047, NOTE_32_MS},// C6
    {0, NOTE_32_MS},   // r
    {-1, -1},         // Terminater
};

static Note *MELODY_SE_LIST[] = {
    MELODY_SE_START,
    MELODY_SE_END,
};

// 音を鳴らす(波形種類・周波数・時間)
void sound_play(const int type, const float freq, const uint32_t duration)
{
    switch (type)
    {
    case SOUND_SQUARE:
        M5.Speaker.tone(freq, duration, 0, true, step_square_wave, sizeof(step_square_wave), false);
        break;
    case SOUND_TRIANGLE:
        M5.Speaker.tone(freq, duration, 1, true, step_triangle_wave, sizeof(step_triangle_wave), false);
        break;
    default:
        break;
    }
}

// SE再生
void sound_play_SE(const SOUND_SE no)
{
    if (no < 0 || no >= 3)
    {
        return;
    }

    Note *m = MELODY_SE_LIST[no];

    while (m->freq != -1)
    {
        sound_play(SOUND_TRIANGLE, m->freq, m->duration);
        delay(m->duration);
        m++;
    }
}
