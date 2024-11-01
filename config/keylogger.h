#pragma once

#define MAX_KEYLOG_SIZE 5

struct keylog_state {
    uint16_t keycodes[MAX_KEYLOG_SIZE];
    uint8_t index;
    bool enabled;
};

static struct keylog_state keylog = {
    .keycodes = {0},
    .index = 0,
    .enabled = false
};k