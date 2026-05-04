/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include "stdint.h"
#include "quantum.h"
#include "led.h"

// [TODO] port this

void led_set_user(led_t led_state)
{
    if (led_state.caps_lock) {
        //led3_indicator_color.r = 255;
        //led3_indicator_color.g = 0;
        //led3_indicator_color.b = 255;
    } else {

    }
}
