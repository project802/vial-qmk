// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0
// Based on the original code by YDKB (unspecified license, but presumably GPL-2.0 or later)

#include <stdint.h>
#include <stdbool.h>

#include "print.h"
#include "util.h"
#include "timer.h"
#include "matrix.h"
#include "debounce.h"
#include "wait.h"

/***************
 * Macros
 */
#define GPIO_MATRIX_DATA_LINPUT     A0
#define GPIO_MATRIX_CLOCK           A1
#define GPIO_MATRIX_RINPUT          A2

#define SERIAL_DATA_CLOCK_PULSE() \
    do { \
        gpio_write_pin_high( GPIO_MATRIX_CLOCK ); \
        asm( "nop" ); \
        asm( "nop" ); \
        gpio_write_pin_low( GPIO_MATRIX_CLOCK ); \
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

__attribute__((weak)) void matrix_init_kb( void ) {
    matrix_init_user(); 
}

__attribute__((weak)) void matrix_scan_kb( void ) {
    matrix_scan_user();
}

__attribute__((weak)) void matrix_init_user( void ) {}

__attribute__((weak)) void matrix_scan_user( void ) {}

matrix_row_t matrix_get_row( uint8_t row )
{
    return matrix[row];
}

void matrix_print( void )
{
    print( "\nr/c 0123456789ABCDEF\n" );
    for( uint8_t row = 0; row < MATRIX_ROWS; row++ ) {
        print_hex8( row );
        print( ": " );
        print_bin_reverse16( matrix_get_row(row) );
        print( "\n" );
    }
}

void matrix_init( void )
{
    gpio_set_pin_output( GPIO_MATRIX_DATA_LINPUT );
    gpio_set_pin_output( GPIO_MATRIX_CLOCK );
    gpio_write_pin_high( GPIO_MATRIX_DATA_LINPUT );
    gpio_write_pin_high( GPIO_MATRIX_CLOCK );
    
    gpio_set_pin_input_high( GPIO_MATRIX_RINPUT );
    
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
 *   - Even (and zero) logical columns read the left PCB (PA0 low = pressed)
 *   - Odd logical columns read the right PCB (PA2 low = pressed) and pulse
 *     the shift clock for next hardware row
 */
uint8_t matrix_scan( void )
{
    static matrix_row_t matrix_raw[MATRIX_ROWS] = {0};
    static uint16_t     matrix_scan_timestamp   = 0;

    bool     changed    = false;
    uint16_t time_check = timer_read();

    // Avoid scanning the matrix faster than 1 kHz to give time for other tasks
    if( matrix_scan_timestamp == time_check ) return 0;
    matrix_scan_timestamp = time_check;

    scan_setup_first_col();

    for( uint8_t row=0; row < MATRIX_ROWS; row++ )
    {
        matrix_row_t row_raw = 0;

        for( uint8_t col=0; col < MATRIX_COLS; col++ )
        {
            uint8_t real_col = col/2;
            if( col & 1 ) real_col += 8;

            kunlun_pcb_t pcb = real_col < 8 ? KUNLUN_PCB_LEFT : KUNLUN_PCB_RIGHT;
            if( scan_is_key_pressed(pcb) )
            {
                row_raw |= (1 << real_col);
            }

            if( real_col >= 8 )
            {
                scan_setup_next_col();

                // This delay is required to ensure that the right keyboard
                // input line has enough time to rise due to the large
                // capacitance on the line. Without this, the right PCB will
                // report phantom key presses. This matches the behavior of
                // the original firmware, which doesn't have this delay but
                // naturally runs much slower.
                wait_us(15);
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
static bool scan_is_key_pressed( kunlun_pcb_t pcb ) {
    if( pcb == KUNLUN_PCB_LEFT )
    {
        return gpio_read_pin( GPIO_MATRIX_DATA_LINPUT ) == 0;
    } 
    else
    {
        return gpio_read_pin( GPIO_MATRIX_RINPUT ) == 0;
    }
}

/*
 * Clock in the keypress test bit (0), which will then get shifted through the 
 * registers used to test the keys one-by-one.
 */
static void scan_setup_first_col( void )
{
    // Setup time is not specified in the data sheet. With a max clock of 
    // 30 MHz and we're running the CPU at 40 MHz, putting in a few NOPs will
    // help ensure it is within margin.
    gpio_set_pin_output( GPIO_MATRIX_DATA_LINPUT );
    gpio_write_pin_low( GPIO_MATRIX_DATA_LINPUT );
    asm( "nop" );
    asm( "nop" );

    // Clock in the first bit
    SERIAL_DATA_CLOCK_PULSE();

    // This returns to high faster than flipping straight to input with a weak 
    // pull-up so we can get to scanning much sooner.
    gpio_write_pin_high( GPIO_MATRIX_DATA_LINPUT );

    // This prepares the line for reading the key state on the left PCB.
    gpio_set_pin_input_high( GPIO_MATRIX_DATA_LINPUT );
}

/*
 * Clock in a 1 bit, which will continue to shift the keypress test bit through
 * the registers.
 */
static void scan_setup_next_col( void )
{
    gpio_set_pin_output( GPIO_MATRIX_DATA_LINPUT );
    gpio_write_pin_high( GPIO_MATRIX_DATA_LINPUT );
    asm( "nop" );
    asm( "nop" );

    SERIAL_DATA_CLOCK_PULSE();

    // This prepares the line for reading the key state on the left PCB.
    gpio_set_pin_input_high( GPIO_MATRIX_DATA_LINPUT );
}
