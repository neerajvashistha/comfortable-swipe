#ifndef __COMFORTABLE_SWIPE__gesture_mouse_hold_gesture__
#define __COMFORTABLE_SWIPE__gesture_mouse_hold_gesture__

/*
Comfortable Swipe
by Rico Tiongson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mouse_hold_gesture.h"
#include <cstdio>   // std::sscanf
#include <cstring>  // strncmp
#include <iostream> // std::cout, std::endl

extern "C" {
#include <xdo.h> // xdo, xdo_new, xdo_free,
                 // xdo_get_mouse_location
                 // CURRENT_WINDOW
}

namespace comfortable_swipe::gesture {
enum {
  MOUSE_NONE = -1,
  MOUSE_MOVE = 0,
  MOUSE_LEFT_CLICK = 1,
  MOUSE_MIDDLE_CLICK = 2,
  MOUSE_RIGHT_CLICK = 3,
  MOUSE_WHEEL_UP = 4,
  MOUSE_WHEEL_DOWN = 5,
  MOUSE_SCROLL = 6,
  MOUSE_SCROLL_REVERSE = 7
};

/**
 * Constructs a new mouse gesture, given "hold3" and "hold4" configurations.
 */
mouse_hold_gesture::mouse_hold_gesture(const char *hold3, const char *hold4)
    : comfortable_swipe::gesture::swipe_gesture(), button(MOUSE_NONE),
      hold3(hold3), hold4(hold4), flag_mousedown(false) {}

/**
 * Destructs this mouse swipe gesture.
 */
mouse_hold_gesture::~mouse_hold_gesture() {}

/**
 * Run mousedown command on hold input.
 */
void mouse_hold_gesture::do_mousedown(const char *mouseinput) {
  const int button = this->button = this->parse_mouse_button(mouseinput);
  if (button != MOUSE_NONE) {
    // eg. MOUSE DOWN hold3 mouse1
    std::printf("MOUSE DOWN hold%d %s\n", this->fingers, mouseinput);
    if (MOUSE_LEFT_CLICK <= button && button <= MOUSE_RIGHT_CLICK) {
      // send mouse down on associated button
      xdo_mouse_down(this->xdo, CURRENTWINDOW, button);
    }
    this->flag_mousedown = true;
  }
}

/**
 * Run mouseup command on hold output.
 */
void mouse_hold_gesture::do_mouseup(const char *mouseinput) {
  const int button = this->button = this->parse_mouse_button(mouseinput);
  if (button != MOUSE_NONE) {
    std::printf("MOUSE UP hold%d %s\n", this->fingers, mouseinput);
    if (MOUSE_LEFT_CLICK <= button && button <= MOUSE_RIGHT_CLICK) {
      // send mouse up on associated button
      xdo_mouse_up(this->xdo, CURRENTWINDOW, button);
    }
    this->flag_mousedown = false;
  }
}

/**
 * Utility method to parse mouse number from input.
 * Returns -1 on failure.
 */
int mouse_hold_gesture::parse_mouse_button(const char *input) {
  // just move without holding button down
  if (std::strcmp(input, "move") == 0)
    return MOUSE_MOVE;

  if (std::strcmp(input, "scroll") == 0)
    return MOUSE_SCROLL;

  if (std::strcmp(input, "scroll_reverse") == 0)
    return MOUSE_SCROLL_REVERSE;

  // get button number
  int button;
  if (std::sscanf(input, "button%d", &button) == 1) {
    if (1 <= button && button <= 6) {
      return button;
    }
  }

  return MOUSE_NONE;
}

/**
 * Hook on begin of mouse swipe gesture.
 */
void mouse_hold_gesture::begin() {
  // call superclass method
  swipe_gesture::begin();
  // dispatch mouse down event
  if (this->fingers == 3) {
    this->do_mousedown(this->hold3);
  } else if (this->fingers == 4) {
    this->do_mousedown(this->hold4);
  }
}

/**
 * Hook on end of mouse swipe gesture.
 */
void mouse_hold_gesture::update() {
  // call superclass method
  swipe_gesture::update();
  if (this->is_mousedown()) {
    if (0 <= this->button && this->button <= 3) {
      // drag mouse with pointer during update
      xdo_move_mouse_relative(this->xdo, this->udx, this->udy);
    } else if (this->button == MOUSE_SCROLL ||
               this->button == MOUSE_SCROLL_REVERSE) {
      // perform naive scroll depending on vertical direction
      int wheel = MOUSE_WHEEL_DOWN;
      if ((this->udy > 0) == (this->button == MOUSE_SCROLL))
        wheel = MOUSE_WHEEL_UP;

      // click wheel on update (note: this is not precise)
      xdo_mouse_down(this->xdo, CURRENTWINDOW, wheel);
      xdo_mouse_up(this->xdo, CURRENTWINDOW, wheel);
    } else if (this->button == MOUSE_WHEEL_UP ||
               this->button == MOUSE_WHEEL_DOWN) {
      // click wheel button on 4 or 5
      xdo_mouse_down(this->xdo, CURRENTWINDOW, this->button);
      xdo_mouse_up(this->xdo, CURRENTWINDOW, this->button);
    }
  }
}

/**
 * Hook on end of swipe gesture.
 */
void mouse_hold_gesture::end() {
  if (this->is_mousedown()) {
    if (this->fingers == 3) {
      this->do_mouseup(this->hold3);
    } else if (this->fingers == 4) {
      this->do_mouseup(this->hold4);
    }
  }

  // call superclass method
  swipe_gesture::end();
}

/**
 * Utility method to check if mouse is current held.
 */
bool mouse_hold_gesture::is_mousedown() const { return this->flag_mousedown; }
} // namespace comfortable_swipe::gesture

#endif /* __COMFORTABLE_SWIPE__gesture_mouse_hold_gesture__ */
