#include <zmk/display.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <lvgl.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

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
    if (!keylog.enabled) {
        LOG_DBG("Keylogger disabled, not logging keycode: %d", keycode);
        return;
    }
    
    LOG_DBG("Adding keycode to log: %d at index: %d", keycode, keylog.index);
    keylog.keycodes[keylog.index] = keycode;
    keylog.index = (keylog.index + 1) % MAX_KEYLOG_SIZE;
}

void keylog_toggle(void) {
    keylog.enabled = !keylog.enabled;
    LOG_DBG("Keylogger toggled: %s", keylog.enabled ? "ON" : "OFF");
    
    if (!keylog.enabled) {
        LOG_DBG("Clearing keylog buffer");
        for (int i = 0; i < MAX_KEYLOG_SIZE; i++) {
            keylog.keycodes[i] = 0;
        }
        keylog.index = 0;
    }
    zmk_display_force_draw();
}

static void draw_keylog(lv_obj_t *parent) {
    if (!keylog.enabled) {
        LOG_DBG("Keylogger disabled, not drawing");
        return;
    }

    LOG_DBG("Drawing keylog to display");
    lv_obj_t *label = lv_label_create(parent);
    static char buf[128];
    char *ptr = buf;

    for (int i = 0; i < MAX_KEYLOG_SIZE; i++) {
        uint8_t idx = (keylog.index + i) % MAX_KEYLOG_SIZE;
        uint16_t keycode = keylog.keycodes[idx];
        
        if (keycode != 0) {
            ptr += sprintf(ptr, "K%d:%04X\n", i+1, keycode);
            LOG_DBG("Adding to display - K%d:%04X", i+1, keycode);
        }
    }

    lv_label_set_text(label, buf);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    LOG_DBG("Finished drawing keylog");
}

static int zmk_widget_status_screen_init(const struct device *dev) {
    LOG_DBG("Initializing status screen widget");
    if (!zmk_display_is_initialized()) {
        LOG_ERR("Display not initialized");
        return -ENODEV;
    }

    lv_obj_t *screen = lv_scr_act();
    draw_keylog(screen);
    LOG_DBG("Status screen widget initialized");

    return 0;
}

int keycode_state_changed_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev->state) {  // Only log on press, not release
        LOG_DBG("Keycode state changed: %d", ev->keycode);
        keylog_add_keycode(ev->keycode);
        zmk_display_force_draw();
    }
    return 0;
}

ZMK_LISTENER(keycode_state_changed_listener, zmk_keycode_state_changed);
ZMK_SUBSCRIPTION(keycode_state_changed_listener, zmk_keycode_state_changed);
SYS_INIT(zmk_widget_status_screen_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);