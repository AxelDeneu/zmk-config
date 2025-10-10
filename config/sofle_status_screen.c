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
    lv_obj_t *obj;
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
    if (level > 80) return "[████]";
    if (level > 60) return "[███░]";
    if (level > 40) return "[██░░]";
    if (level > 20) return "[█░░░]";
    return "[░░░░]";
}

// Bluetooth connection indicators
static const char *get_bluetooth_status(int profile, bool usb_connected) {
    if (usb_connected) {
        return "USB";
    }

    static char buf[20];
    char indicators[6] = "○○○○○";

    // Mark active profile with ●
    if (profile >= 0 && profile < 5) {
        indicators[profile] = '●';
    }

    snprintf(buf, sizeof(buf), "BT:[%d]%s", profile + 1, indicators);
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
            lv_label_set_text_fmt(widget->battery_label, "BAT: %s %d%% CHG",
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
    state.battery_level = ev->state_of_charge;
    state.battery_charging = ev->state == ZMK_USB_CONN_CHARGING;

    struct custom_widget *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_display(widget);
    }
    return 0;
}

static int handle_usb_conn_state_changed(const zmk_event_t *eh) {
    const struct zmk_usb_conn_state_changed *ev = as_zmk_usb_conn_state_changed(eh);
    state.usb_connected = (ev->conn_state == ZMK_USB_CONN_HID);

    struct custom_widget *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        update_display(widget);
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
static int custom_widget_init(struct zmk_widget_status *widget) {
    struct custom_widget *cw = (struct custom_widget *)widget;

    cw->obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cw->obj, 128, 64);

    // Layer label
    cw->layer_label = lv_label_create(cw->obj);
    lv_obj_align(cw->layer_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(cw->layer_label, "LAYER: AZERTY");

    // Bluetooth label
    cw->bluetooth_label = lv_label_create(cw->obj);
    lv_obj_align(cw->bluetooth_label, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_label_set_text(cw->bluetooth_label, "BT:[1]●○○○○");

    // Battery label
    cw->battery_label = lv_label_create(cw->obj);
    lv_obj_align(cw->battery_label, LV_ALIGN_TOP_LEFT, 0, 32);
    lv_label_set_text(cw->battery_label, "BAT: [████] 100%");

    sys_slist_append(&widgets, &cw->node);

    // Initial update
    update_display(cw);

    return 0;
}

// Cleanup
static int custom_widget_cleanup(struct zmk_widget_status *widget) {
    struct custom_widget *cw = (struct custom_widget *)widget;
    sys_slist_find_and_remove(&widgets, &cw->node);
    lv_obj_del(cw->obj);
    return 0;
}

// Widget definition
static const struct zmk_widget_status_def custom_widget_def = {
    .init = custom_widget_init,
    .cleanup = custom_widget_cleanup,
};

ZMK_DISPLAY_WIDGET_LISTENER(widget_custom_status, &custom_widget_def);
ZMK_SUBSCRIPTION(widget_custom_status, zmk_layer_state_changed);
