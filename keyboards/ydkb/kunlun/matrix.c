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

typedef enum {
    KUNLUN_PCB_LEFT = 0,
    KUNLUN_PCB_RIGHT = 1,
} kunlun_pcb_t;

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
static uint8_t get_key(kunlun_pcb_t pcb);
static void scan_setup_first_col( void );
static void scan_setup_next_col( void );

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
 * - Shift in a single 0 bit to the first register on both left and right PCBs
 * - Iterate the columns within the rows in a loop:
 *   - Even (and zero) columns read the selected key on the left PCB (PB3 low = pressed)
 *   - Odd columns read the selected key on the right PCB (PB2 low = pressed) and pulse the shift clock
 */
uint8_t matrix_scan(void)
{
    // [TODO] use standard debouncing?

    uint16_t time_check = timer_read();
    if (matrix_scan_timestamp == time_check) return 1;
    matrix_scan_timestamp = time_check;

    scan_setup_first_col();
    uint8_t *debounce = &matrix_debouncing[0][0];
    for (uint8_t row=0; row<MATRIX_ROWS; row++) {
        for (uint8_t col=0; col<MATRIX_COLS; col++, *debounce++) {
            uint8_t real_col = col/2;
            if (col & 1) real_col += 8;

            uint8_t key = get_key(real_col < 8 ? KUNLUN_PCB_LEFT : KUNLUN_PCB_RIGHT);

            if (real_col >= 8) scan_setup_next_col();

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

static uint8_t get_key(kunlun_pcb_t pcb) {
    if (pcb == KUNLUN_PCB_LEFT)
    {
        return PINB & (1<<PB3) ? 0 : 0x80;
    } 
    else
    {
        return PINB & (1<<PB2) ? 0 : 0x80;
    }
}

/*
 * Clock in the keypress test bit (a 0), which will then get shifted through the registers
 * used to test the keys one-by-one.
 */
static void scan_setup_first_col( void )
{
    // PB3 | serial data = output, low
    //
    // Setup time is not specified in the data sheet, but with a max clock of 30 MHz
    // (cycle time of 33 ns) and a single nop is about 150 us, this should have enough margin.
    DDRB |= (1<<PB3);
    PORTB &= ~(1<<PB3);
    asm("nop");
    asm("nop");

    // Clock in the first bit
    SERIAL_DATA_CLOCK_PULSE();

    // PB3 = high
    // This returns to high faster than flipping straight to input with a weak pull-up
    // so we can get to scanning much sooner.
    PORTB |= (1<<PB3);

    // PB3 = input (and pull-up enabled by previous line)
    // This prepares the line for reading the key state on the left PCB
    DDRB &= ~(1<<PB3);
}

/*
 * Clock in a 1 bit, which will continue to shift the keypress test bit through the registers.
 */
static void scan_setup_next_col( void )
{
    // PB3 | serial data = output, high
    DDRB |= (1<<PB3);

    SERIAL_DATA_CLOCK_PULSE();

    // PB3 = input (and pull-up enabled previously)
    DDRB &= ~(1<<PB3);
}
