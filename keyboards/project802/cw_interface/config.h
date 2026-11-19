// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0

#pragma once

#define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE

#define MATRIX_ROWS 1
#define MATRIX_COLS 2

#define CW_RIGHT_PIN B6
#define CW_LEFT_PIN B7
#define DEBOUNCE 30

#ifdef AUDIO_ENABLE
#    define AUDIO_PIN A5
#    define AUDIO_PIN_ALT A4
#    define AUDIO_PIN_ALT_AS_NEGATIVE
#    define AUDIO_PWM_DRIVER PWMD3
#    define AUDIO_PWM_CHANNEL 1
#    define AUDIO_STATE_TIMER GPTD8

#    include "startup_song.h"
#    define TEMPO_DEFAULT CW_STARTUP_TEMPO
#endif
