// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0

#include "matrix.h"

void matrix_init_custom(void) {
    // The XIAO RP2040's onboard RGB LED channels are active-low.
    gpio_write_pin_high(GP17);
    gpio_set_pin_output(GP17);
    gpio_write_pin_high(GP16);
    gpio_set_pin_output(GP16);
    gpio_write_pin_high(GP25);
    gpio_set_pin_output(GP25);

    gpio_set_pin_input_high(DIT_PIN);
    gpio_set_pin_input_high(DAH_PIN);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    matrix_row_t row = 0;
    if (!gpio_read_pin(DIT_PIN)) {
        row |= MATRIX_ROW_SHIFTER << 0;
    }
    if (!gpio_read_pin(DAH_PIN)) {
        row |= MATRIX_ROW_SHIFTER << 1;
    }

    bool changed = current_matrix[0] != row;
    current_matrix[0] = row;
    return changed;
}
