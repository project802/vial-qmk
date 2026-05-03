#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "wait.h"
#include "action_layer.h"
#include "print.h"
#include "debug.h"
#include "util.h"
#include "timer.h"
#include "matrix.h"
#include "switch_board.h"
#include "rgblight.h"

/*
 * Matrix state buffer (1:on, 0:off)
 */
static matrix_row_t matrix[MATRIX_ROWS] = {0};

/*
 * Debouncing definitions
 */
#define DEBOUNCE_DN_MASK (uint8_t)(~(0x80 >> 5))
#define DEBOUNCE_UP_MASK (uint8_t)(0x80 >> 5)

static uint16_t matrix_scan_timestamp = 0;
static uint8_t matrix_debouncing[MATRIX_ROWS][MATRIX_COLS] = {0};

/*
 * Matrix scanning support functions
 */
static void select_next_key(uint8_t mode);
static uint8_t get_key(uint8_t col);


/*
 * Required functions for custom matrix.
 * 
 * See https://docs.qmk.fm/custom_matrix#full-replacement
 */

__attribute__((weak)) void matrix_init_kb(void) {
    matrix_init_user(); 
}

__attribute__((weak)) void matrix_scan_kb(void) {
    matrix_scan_user();
    hook_keyboard_loop();
}

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        print_hex8(row); print(": ");
        print_bin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

void matrix_init(void)
{
    
    /*
     * PB1 = serial data clock
     * PB2 = right PCB keypress input
     * PB3 = serial data and left PCB keypress input
     */

    // Set PB1 and PB3 as output, PB2 as input with pull-up
    DDRB  |=  (1<<PB3 | 1<<PB1);
    DDRB  &= ~(1<<PB2);
    PORTB |=  (1<<PB3 | 1<<PB2 | 1<<PB1);

    rgblight_init();
}

/*
 * Scans the matrix and updates the state.
 *
 * - Flush the shift registers with a series of clock pulses
 * - Shift in a single 0 bit to the first register on both left and right PCBs
 * - Iterate the columns within the rows in a loop:
 *   - Even (and zero) columns read the left PCB (PB3 low = pressed)
 *   - Odd columns read the right PCB (PB2 low = pressed) and then pulse the shift clock
 */
uint8_t matrix_scan(void)
{
    // [TODO] use standard debouncing?

    uint16_t time_check = timer_read();
    if (matrix_scan_timestamp == time_check) return 1;
    matrix_scan_timestamp = time_check;

    select_next_key(0);
    uint8_t *debounce = &matrix_debouncing[0][0];
    for (uint8_t row=0; row<MATRIX_ROWS; row++) {
        for (uint8_t col=0; col<MATRIX_COLS; col++, *debounce++) {
            uint8_t real_col = col/2;
            if (col & 1) real_col += 8;

            uint8_t key = get_key(real_col);
            if (real_col >= 8) select_next_key(1);
            *debounce = (*debounce >> 1) | key;
            if ((*debounce > 0) && (*debounce < 255)) {
                matrix_row_t *p_row = &matrix[row];
                matrix_row_t col_mask = ((matrix_row_t)1 << real_col);
                if        (*debounce >= DEBOUNCE_DN_MASK) {
                    *p_row |=  col_mask;
                } else if (*debounce <= DEBOUNCE_UP_MASK) {
                    *p_row &= ~col_mask;
                }
            } 
        }
    }
    
    // Must call matrix_scan_kb to ensure QMK execution order compliance
    matrix_scan_kb();

    // Return value doesn't mean anything
    return 1;
}

/*
 * Kunlun-specific functions to support matrix scanning.
 */

static uint8_t get_key(uint8_t col) {
    if (col<8) return PINB&(1<<PB3) ? 0 : 0x80;
    else return PINB&(1<<PB2) ? 0 : 0x80;
}

static void select_next_key(uint8_t mode)
{
    // PB3 = output
    DDRB |= (1<<PB3);
    
    if (mode == 0) {
        DS_PL_HI();
        for (uint8_t i = 0; i < 40; i++) {
            CLOCK_PULSE();
        }
        DS_PL_LO();
        CLOCK_PULSE();
    } else {
        DS_PL_HI();
        CLOCK_PULSE();
    }
    
    // PB3 = input with pull-up
    DDRB  &= ~(1<<PB3);
    PORTB |=  (1<<PB3);
    _delay_us(5);
}

