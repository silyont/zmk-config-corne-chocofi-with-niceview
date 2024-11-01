#include <zmk/display.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <lvgl.h>

#define MAX_KEYLOG_SIZE 5

static struct {
    uint16_t keycodes[MAX_KEYLOG_SIZE];
    uint8_t index;
    bool enabled;
} keylog = {
    .keycodes = {0},
    .index = 0,
    .enabled = false
};

static void keylog_add_keycode(uint16_t keycode) {
    if (!keylog.enabled) return;
    
    keylog.keycodes[keylog.index] = keycode;
    keylog.index = (keylog.index + 1) % MAX_KEYLOG_SIZE;
}

void keylog_toggle(void) {
    keylog.enabled = !keylog.enabled;
    if (!keylog.enabled) {
        // Clear log when disabled
        for (int i = 0; i < MAX_KEYLOG_SIZE; i++) {
            keylog.keycodes[i] = 0;
        }
        keylog.index = 0;
    }
    zmk_display_force_draw();
}

static void draw_keylog(lv_obj_t *parent) {
    if (!keylog.enabled) return;

    lv_obj_t *label = lv_label_create(parent);
    static char buf[128];
    char *ptr = buf;

    for (int i = 0; i < MAX_KEYLOG_SIZE; i++) {
        uint8_t idx = (keylog.index + i) % MAX_KEYLOG_SIZE;
        uint16_t keycode = keylog.keycodes[idx];
        
        if (keycode != 0) {
            ptr += sprintf(ptr, "K%d:%04X\n", i+1, keycode);
        }
    }

    lv_label_set_text(label, buf);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}

static int zmk_widget_status_screen_init(const struct device *dev) {
    if (!zmk_display_is_initialized()) {
        return -ENODEV;
    }

    lv_obj_t *screen = lv_scr_act();
    draw_keylog(screen);

    return 0;
}

int keycode_state_changed_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev->state) {  // Only log on press, not release
        keylog_add_keycode(ev->keycode);
        zmk_display_force_draw();
    }
    return 0;
}

ZMK_LISTENER(keycode_state_changed_listener, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(keycode_state_changed_listener, zmk_keycode_state_changed);
SYS_INIT(zmk_widget_status_screen_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);