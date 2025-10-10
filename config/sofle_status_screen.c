/*
 * Copyright (c) 2024 Custom OLED Status Screen
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/battery.h>

#include <lvgl.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// Widget state structure
struct status_state {
    uint8_t battery_level;
    bool battery_charging;
    bool usb_connected;
    int active_profile;
    uint8_t active_layer;
};

static struct status_state state = {
    .battery_level = 0,
    .battery_charging = false,
    .usb_connected = false,
    .active_profile = 0,
    .active_layer = 0,
};

struct custom_widget {
    sys_snode_t node;
    lv_obj_t *layer_label;
    lv_obj_t *battery_label;
    lv_obj_t *bluetooth_label;
};

// Layer names in French
static const char *get_layer_name(uint8_t layer) {
    switch (layer) {
        case 0: return "AZERTY";
        case 1: return "SYMBOLES";
        case 2: return "RAISE";
        case 3: return "ADJUST";
        default: return "UNKNOWN";
    }
}

// Battery icon based on level
static const char *get_battery_icon(uint8_t level) {
    if (level > 80) return "[####]";
    if (level > 60) return "[### ]";
    if (level > 40) return "[##  ]";
    if (level > 20) return "[#   ]";
    return "[    ]";
}

// Bluetooth connection indicators
static const char *get_bluetooth_status(int profile, bool usb_connected) {
    if (usb_connected) {
        return "USB";
    }

    static char buf[20];
    const char *dot_filled = "#";
    const char *dot_empty = "o";
    char indicators[16];

    // Create indicator string
    snprintf(indicators, sizeof(indicators), "%s%s%s%s%s",
             profile == 0 ? dot_filled : dot_empty,
             profile == 1 ? dot_filled : dot_empty,
             profile == 2 ? dot_filled : dot_empty,
             profile == 3 ? dot_filled : dot_empty,
             profile == 4 ? dot_filled : dot_empty);

    snprintf(buf, sizeof(buf), "BT:%d %s", profile + 1, indicators);
    return buf;
}

// Update display
static void update_display(struct custom_widget *widget) {
    if (widget->layer_label != NULL) {
        lv_label_set_text_fmt(widget->layer_label, "LAYER: %s",
                              get_layer_name(state.active_layer));
    }

    if (widget->battery_label != NULL) {
        if (state.battery_charging) {
            lv_label_set_text_fmt(widget->battery_label, "BAT: %s %d%% +",
                                  get_battery_icon(state.battery_level),
                                  state.battery_level);
        } else {
            lv_label_set_text_fmt(widget->battery_label, "BAT: %s %d%%",
                                  get_battery_icon(state.battery_level),
                                  state.battery_level);
        }
    }

    if (widget->bluetooth_label != NULL) {
        lv_label_set_text(widget->bluetooth_label,
                         get_bluetooth_status(state.active_profile, state.usb_connected));
    }
}

// Event handlers
static int handle_battery_state_changed(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    if (ev != NULL) {
        state.battery_level = ev->state_of_charge;
        state.battery_charging = (ev->state == ZMK_USB_CONN_CHARGING);

        struct custom_widget *widget;
        SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
            update_display(widget);
        }
    }
    return 0;
}

static int handle_usb_conn_state_changed(const zmk_event_t *eh) {
    const struct zmk_usb_conn_state_changed *ev = as_zmk_usb_conn_state_changed(eh);
    if (ev != NULL) {
        state.usb_connected = (ev->conn_state == ZMK_USB_CONN_HID);

        struct custom_widget *widget;
        SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
            update_display(widget);
        }
    }
    return 0;
}

static int handle_ble_profile_changed(const zmk_event_t *eh) {
    state.active_profile = zmk_ble_active_profile_index();

    struct custom_widget *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_display(widget);
    }
    return 0;
}

static int handle_layer_state_changed(const zmk_event_t *eh) {
    state.active_layer = zmk_keymap_highest_layer_active();

    struct custom_widget *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_display(widget);
    }
    return 0;
}

ZMK_LISTENER(widget_battery_status, handle_battery_state_changed);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);

ZMK_LISTENER(widget_usb_status, handle_usb_conn_state_changed);
ZMK_SUBSCRIPTION(widget_usb_status, zmk_usb_conn_state_changed);

ZMK_LISTENER(widget_ble_status, handle_ble_profile_changed);
ZMK_SUBSCRIPTION(widget_ble_status, zmk_ble_active_profile_changed);

ZMK_LISTENER(widget_layer_status, handle_layer_state_changed);
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

// Initialize widget
static lv_obj_t *widget_init(lv_obj_t *parent) {
    struct custom_widget *widget = k_malloc(sizeof(struct custom_widget));
    if (widget == NULL) {
        LOG_ERR("Failed to allocate custom widget");
        return NULL;
    }

    // Layer label
    widget->layer_label = lv_label_create(parent);
    lv_obj_align(widget->layer_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(widget->layer_label, "LAYER: AZERTY");

    // Bluetooth label
    widget->bluetooth_label = lv_label_create(parent);
    lv_obj_align(widget->bluetooth_label, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_label_set_text(widget->bluetooth_label, "BT:1 #oooo");

    // Battery label
    widget->battery_label = lv_label_create(parent);
    lv_obj_align(widget->battery_label, LV_ALIGN_TOP_LEFT, 0, 32);
    lv_label_set_text(widget->battery_label, "BAT: [####] 100%");

    sys_slist_append(&widgets, &widget->node);

    // Get initial battery state
#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
    state.battery_level = zmk_battery_state_of_charge();
#endif
    state.active_profile = zmk_ble_active_profile_index();
    state.active_layer = zmk_keymap_highest_layer_active();

    // Initial update
    update_display(widget);

    return widget->layer_label;
}

// ZMK display widget registration
static struct zmk_display_widget_status_screen {
    lv_obj_t *(*init)(lv_obj_t *parent);
} custom_status_screen = {
    .init = widget_init,
};

static int custom_status_screen_init(void) {
    zmk_display_widget_status_screen_init(&custom_status_screen);
    return 0;
}

SYS_INIT(custom_status_screen_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
