#include <linux/input.h>

#include "recovery_ui.h"
#include "common.h"
#include "extendedcommands.h"


int device_toggle_display(volatile char* key_pressed, int key_code) {
    int alt = key_pressed[KEY_VOLUMEUP] && key_pressed[KEY_VOLUMEDOWN];
    if (alt && (key_code == KEY_VOLUMEUP || key_code == KEY_VOLUMEDOWN))
        return 1;
    // allow toggling of the display if the correct key is pressed, and the display toggle is allowed or the display is currently off
    if (ui_get_showing_back_button())
        return 0;
    return get_allow_toggle_display() && key_code == KEY_POWER;
}

int device_handle_key(int key_code, int visible) {
    if (visible) {
        if (!ui_get_showing_back_button()) {
            ui_print("Back menu button enabled.\n");
            ui_set_showing_back_button(1);
            return NO_ACTION;
        }
        switch (key_code) {
            case KEY_VOLUMEUP:
                return HIGHLIGHT_UP;
            case KEY_VOLUMEDOWN:
                return HIGHLIGHT_DOWN;
            case KEY_POWER:
                return SELECT_ITEM;
        }
    }
    return NO_ACTION;
}
