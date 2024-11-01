#include <zmk/display/widgets/peripheral_status.h>
#include "keylogger.h"

static void peripheral_status_update_cb(struct zmk_widget_peripheral_status *widget) {
    if (!keylog.enabled) {
        // Show default peripheral status
        widget->show_art = true;
        return;
    }

    // Clear default art
    widget->show_art = false;
    
    // Update with keylog data
    char text[32];
    for (int i = 0; i < MAX_KEYLOG_SIZE; i++) {
        uint8_t idx = (keylog.index + i) % MAX_KEYLOG_SIZE;
        uint16_t keycode = keylog.keycodes[idx];
        
        if (keycode != 0) {
            snprintf(text, sizeof(text), "K%d: %04X", i+1, keycode);
            lv_label_set_text(widget->label, text);
        }
    }
}

static struct zmk_widget_peripheral_status peripheral_status_widget = {
    .update_cb = peripheral_status_update_cb
};