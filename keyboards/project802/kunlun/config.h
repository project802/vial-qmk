// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0
// Based on the original code by YDKB (unspecified license, but presumably GPL-2.0 or later)

#pragma once

/* Ensure we jump to bootloader if the QK_BOOT keycode was pressed */
#define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE

/* key matrix size */
#define MATRIX_ROWS 5
#define MATRIX_COLS 16

#define AUDIO_CLICKY
#define AUDIO_PIN A8
#define AUDIO_PWM_DRIVER PWMD1
#define AUDIO_PWM_CHANNEL 1
#define AUDIO_STATE_TIMER GPTD3
