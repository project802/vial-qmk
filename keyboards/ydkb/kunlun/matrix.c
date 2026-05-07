// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0
// Based on the original code by YDKB (unspecified license, but presumably GPL-2.0 or later)

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "print.h"
#include "util.h"
#include "timer.h"
#include "matrix.h"
#include "debounce.h"

/***************
 * Macros
 */
#define SERIAL_DATA_CLOCK_PULSE() \
    do { \
        PORTB |= (1<<PB1); \
        asm("nop"); \
        PORTB &= ~(1<<PB1); \
    } while(0)


/***************
 * Matrix scanning definitions
 */
typedef enum {
    KUNLUN_PCB_LEFT = 0,
    KUNLUN_PCB_RIGHT = 1,
} kunlun_pcb_t;

static bool scan_is_key_pressed( kunlun_pcb_t pcb );
static void scan_setup_first_col( void );
static void scan_setup_next_col( void );

// Matrix debounced state buffer (1:pressed, 0:released)
static matrix_row_t matrix[MATRIX_ROWS] = {0};

/***************
 * Required functions for custom matrix.
 * 
 * See https://docs.qmk.fm/custom_matrix#full-replacement
 */

__attribute__((weak)) void matrix_init_kb(void) {
    matrix_init_user(); 
}

__attribute__((weak)) void matrix_scan_kb(void) {
    matrix_scan_user();
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
     * PB3 = serial data output and left PCB keypress input
     */

    // Set PB1 and PB3 as output, PB2 as input with pull-up
    DDRB  |=  (1<<PB3 | 1<<PB1);
    DDRB  &= ~(1<<PB2);
    PORTB |=  (1<<PB3 | 1<<PB2 | 1<<PB1);

    debounce_init( MATRIX_ROWS );
}

/*
 * Scans the matrix and updates the state.
 *
 * The keyboard hardware is actually 40 rows (max number of outputs in the shift
 * register serial arrangement) and 2 columns (each PCB), but QMK doesn't handle
 * 40 rows efficiently.
 * 
 * The original designers seem to have decided to use the concept of "real column"
 * to translate the QMK logical matrix to the hardware matrix.
 *
 * - Shift in a single 0 bit to the first register output on both left and 
 *   right PCBs. This sets up the first hardware row for evaluation.
 * - Iterate the logical columns within the logical rows in a loop:
 *   - Even (and zero) logical columns read the left PCB (PB3 low = pressed)
 *   - Odd logical columns read the right PCB (PB2 low = pressed) and pulse
 *     the shift clock for next hardware row
 */
uint8_t matrix_scan(void)
{
    static matrix_row_t matrix_raw[MATRIX_ROWS] = {0};
    static uint16_t     matrix_scan_timestamp   = 0;

    bool     changed    = false;
    uint16_t time_check = timer_read();

    // Avoid scanning the matrix faster than 1 kHz to give time for other tasks
    if (matrix_scan_timestamp == time_check) return 0;
    matrix_scan_timestamp = time_check;

    scan_setup_first_col();

    for (uint8_t row=0; row<MATRIX_ROWS; row++)
    {
        matrix_row_t row_raw = 0;

        for (uint8_t col=0; col<MATRIX_COLS; col++)
        {
            uint8_t real_col = col/2;
            if (col & 1) real_col += 8;

            kunlun_pcb_t pcb = real_col < 8 ? KUNLUN_PCB_LEFT : KUNLUN_PCB_RIGHT;
            if( scan_is_key_pressed(pcb) )
            {
                row_raw |= (1 << real_col);
            }

            if (real_col >= 8)
            {
                scan_setup_next_col();
            }
        }

        if( matrix_raw[row] != row_raw )
        {
            matrix_raw[row] = row_raw;
            changed = true;
        }
    }

    debounce( matrix_raw, matrix, MATRIX_ROWS, changed );
    
    // Must call matrix_scan_kb to ensure QMK execution order compliance
    matrix_scan_kb();

    return changed;
}

/***************
 * Functions to support matrix scanning.
 */

inline bool matrix_is_on( uint8_t row, uint8_t col )
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

/*
 * Read the current key state depending on which PCB is selected.
 *
 * The shift register drives low on the hardware column under test so pressing 
 * the corresponding switch will pull either the left PCB input (PB3) or the 
 * right PCB input (PB2) low.
 */
static bool scan_is_key_pressed(kunlun_pcb_t pcb) {
    if (pcb == KUNLUN_PCB_LEFT)
    {
        return (PINB & (1<<PB3)) == 0;
    } 
    else
    {
        return (PINB & (1<<PB2)) == 0;
    }
}

/*
 * Clock in the keypress test bit (0), which will then get shifted through the 
 * registers used to test the keys one-by-one.
 */
static void scan_setup_first_col( void )
{
    // PB3 | serial data = output, low
    //
    // Setup time is not specified in the data sheet, but with a max clock of 
    // 30 MHz (33 ns) and a single nop is about 150 us, this should have
    // enough margin.
    DDRB |= (1<<PB3);
    PORTB &= ~(1<<PB3);
    asm("nop");
    asm("nop");

    // Clock in the first bit
    SERIAL_DATA_CLOCK_PULSE();

    // PB3 = high
    // This returns to high faster than flipping straight to input with a weak 
    // pull-up so we can get to scanning much sooner.
    PORTB |= (1<<PB3);

    // PB3 = input (and pull-up enabled by previous line)
    // This prepares the line for reading the key state on the left PCB.
    DDRB &= ~(1<<PB3);
}

/*
 * Clock in a 1 bit, which will continue to shift the keypress test bit through
 * the registers.
 */
static void scan_setup_next_col( void )
{
    // PORTB bit for PB3 is already set to 1 once the scan has started, so
    // there is no need to set it again here.

    // Why not keep PB3 as an input with pull-up? If the wrong key is pressed,
    // the shift register output will pull PB3 low and we will clock in the 
    // wrong bit. The reason why driving it high works is that even with the 
    // wrong key pressed (which would happen if you hold down anything on the 
    // left PCB for even the shortest duration) is that the resistor on the left
    // PCB will prevent a tug-of-war.

    // PB3 | serial data = output, high
    DDRB |= (1<<PB3);

    SERIAL_DATA_CLOCK_PULSE();

    // PB3 = input, pull-up
    DDRB &= ~(1<<PB3);
}
