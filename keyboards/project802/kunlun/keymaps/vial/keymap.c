// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0
// Based on the original code by YDKB (unspecified license, but presumably GPL-2.0 or later)

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

const rgblight_segment_t PROGMEM rgblight_caps_lock_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 3, HSV_PURPLE}
);

const rgblight_segment_t PROGMEM rgblight_layer1_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 3, HSV_BLUE}
);

const rgblight_segment_t PROGMEM rgblight_layer2_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 3, HSV_RED}
);

const rgblight_segment_t PROGMEM rgblight_layer3_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 3, HSV_GREEN}
);

// Layer priority is last to first
const rgblight_segment_t* const PROGMEM kunlun_rgblight_layers[] = RGBLIGHT_LAYERS_LIST(
	rgblight_layer1_layer,
	rgblight_layer2_layer,
	rgblight_layer3_layer,
	rgblight_caps_lock_layer
);

void keyboard_post_init_user( void )
{
	rgblight_layers = kunlun_rgblight_layers;
}

bool led_update_user( led_t led_state )
{
	// First argument is the index of kunlun_rgblight_layers[]
    rgblight_set_layer_state( 3, led_state.caps_lock );
    return true;
}

layer_state_t layer_state_set_user( layer_state_t state )
{
	// First argument is the index of kunlun_rgblight_layers[]
	rgblight_set_layer_state( 0, layer_state_cmp(state, 1) );
    rgblight_set_layer_state( 1, layer_state_cmp(state, 2) );
    rgblight_set_layer_state( 2, layer_state_cmp(state, 3) );
    return state;
}
