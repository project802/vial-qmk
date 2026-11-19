// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0

#include "matrix.h"

void matrix_init_custom(void) {
    gpio_set_pin_input_high(CW_LEFT_PIN);
    gpio_set_pin_input_high(CW_RIGHT_PIN);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    matrix_row_t row = 0;
    if (!gpio_read_pin(CW_LEFT_PIN)) {
        row |= MATRIX_ROW_SHIFTER << 0;
    }
    if (!gpio_read_pin(CW_RIGHT_PIN)) {
        row |= MATRIX_ROW_SHIFTER << 1;
    }

    bool changed = current_matrix[0] != row;
    current_matrix[0] = row;
    return changed;
}
