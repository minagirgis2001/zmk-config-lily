/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/display.h>
#include <zmk/display/widgets/bongo_cat.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct bongo_state {
    uint8_t animation_frame;
    bool is_typing;
    int64_t last_keypress;
};

static struct bongo_state bongo_state = {
    .animation_frame = 0,
    .is_typing = false,
    .last_keypress = 0,
};

// Simple bongo cat frames (basic ASCII art for now)
static const char *bongo_frames[] = {
    "   /\\_/\\  \n"
    "  ( o.o ) \n"
    "   > ^ <  \n"
    " Sleeping \n",
    
    "   /\\_/\\  \n"
    "  ( ^.^ ) \n"
    "  _| - |_ \n"
    " Typing!  \n",
    
    "   /\\_/\\  \n"
    "  ( >.< ) \n"
    "  _|   |_ \n"
    " Bongoing!\n"
};

static void set_bongo_state(struct zmk_widget_bongo_cat *widget) {
    int64_t current_time = k_uptime_get();
    int64_t time_since_last = current_time - bongo_state.last_keypress;
    
    // Animation logic based on recent activity
    if (time_since_last < 1000) {
        bongo_state.animation_frame = 2; // Fast bongoing
        bongo_state.is_typing = true;
    } else if (time_since_last < 5000) {
        bongo_state.animation_frame = 1; // Slow typing
        bongo_state.is_typing = true;
    } else {
        bongo_state.animation_frame = 0; // Idle/sleeping
        bongo_state.is_typing = false;
    }
}

static void draw_bongo_cat(lv_obj_t *canvas, struct bongo_state *state) {
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
    
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_white();
    
    lv_point_t pos = {.x = 0, .y = 0};
    
    lv_canvas_draw_text(canvas, 0, 0, 120, &label_dsc, 
                       bongo_frames[state->animation_frame]);
}

static void bongo_cat_update_cb(struct zmk_widget_bongo_cat *widget) {
    set_bongo_state(widget);
    draw_bongo_cat(widget->obj, &bongo_state);
}

static int bongo_cat_listener(const zmk_event_t *eh) {
    struct zmk_widget_bongo_cat *widget;
    
    // Handle position state changes (key presses)
    if (as_zmk_position_state_changed(eh) != NULL) {
        struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
        if (ev->state) { // Key pressed
            bongo_state.last_keypress = k_uptime_get();
        }
        
        SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
            bongo_cat_update_cb(widget);
        }
    }
    
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_bongo_cat, bongo_cat_listener);
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_position_state_changed);

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    widget->obj = lv_canvas_create(parent);
    lv_obj_align(widget->obj, LV_ALIGN_CENTER, 0, 0);
    
    sys_slist_append(&widgets, &widget->node);
    
    // Initial draw
    bongo_cat_update_cb(widget);
    
    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget->obj;
}