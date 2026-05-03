#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT( 
	KC_ESC,  KC_1,   KC_2,   KC_3,    KC_4,    KC_5,    KC_6,   KC_TAB,  KC_RGHT, KC_SPC, KC_RALT, KC_LEFT, KC_HOME, KC_PSCR, KC_DOWN,
	KC_Q,    KC_W,   KC_E,   KC_R,    KC_T,    KC_G,    KC_F,   KC_CAPS, KC_UP,   KC_B,   KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
	KC_A,    KC_S,   KC_D,   KC_B,    KC_V,    KC_C,    KC_X,   KC_LSFT, KC_ENT,  KC_H,   KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_END,
	KC_Z,    KC_DEL, KC_SPC, KC_LALT, KC_LCTL, KC_BSLS, KC_Y,   KC_U,    KC_I,    KC_O,   KC_P,    KC_LBRC, KC_RBRC,  
	KC_BSPC, KC_7,   KC_8,   KC_9,    KC_0,    KC_MINS, KC_EQL, KC_GRV 
	),
};