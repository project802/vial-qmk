// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0

#include "debounce.h"
#include "timer.h"
#include <string.h>

static uint32_t release_started[MATRIX_ROWS][MATRIX_COLS];
static matrix_row_t release_pending[MATRIX_ROWS];

void debounce_init(uint8_t num_rows) {
    memset(release_started, 0, sizeof(release_started));
    memset(release_pending, 0, sizeof(release_pending));
}

void debounce_free(void) {}

bool debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed) {
    bool cooked_changed = false;
    uint32_t now = timer_read32();

    for (uint8_t row = 0; row < num_rows; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            matrix_row_t mask = MATRIX_ROW_SHIFTER << col;
            if (raw[row] & mask) {
                release_pending[row] &= ~mask;
                if (!(cooked[row] & mask)) {
                    cooked[row] |= mask;
                    cooked_changed = true;
                }
            } else if (cooked[row] & mask) {
                if (!(release_pending[row] & mask)) {
                    release_pending[row] |= mask;
                    release_started[row][col] = now;
                } else if (TIMER_DIFF_32(now, release_started[row][col]) >= DEBOUNCE) {
                    cooked[row] &= ~mask;
                    release_pending[row] &= ~mask;
                    cooked_changed = true;
                }
            }
        }
    }

    return cooked_changed;
}
