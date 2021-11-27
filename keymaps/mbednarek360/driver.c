#include QMK_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

enum ctrl_keycodes {
    L_BRI = SAFE_RANGE, // LED Brightness Increase
    L_BRD,              // LED Brightness Decrease
    L_PTN,              // LED Pattern Select Next
    L_PTP,              // LED Pattern Select Previous
    L_PSI,              // LED Pattern Speed Increase
    L_PSD,              // LED Pattern Speed Decrease
    L_T_MD,             // LED Toggle Mode
    L_T_ONF,            // LED Toggle On / Off
    L_ON,               // LED On
    L_OFF,              // LED Off
    L_T_BR,             // LED Toggle Breath Effect
    L_T_PTD,            // LED Toggle Scrolling Pattern Direction
    U_T_AGCR,           // USB Toggle Automatic GCR control
    DBG_TOG,            // DEBUG Toggle On / Off
    DBG_MTRX,           // DEBUG Toggle Matrix prints
    DBG_KBD,            // DEBUG Toggle Keyboard prints
    DBG_MOU,            // DEBUG Toggle Mouse prints
    MD_BOOT             // Restart into bootloader after hold timeout
};
       
typedef union {
  uint32_t raw;
  struct {
    uint8_t led_animation_id: 3,
            led_lighting_mode: 2,
            led_animation_breathing: 1,
            led_enabled: 1,
            led_animation_direction: 1;
    uint8_t gcr_desired;
    uint8_t led_animation_speed;
    uint8_t _unused;
  };
} kb_config_t;

kb_config_t kb_config;

void load_saved_settings(void) {
    kb_config.raw = eeconfig_read_kb();

    led_animation_id = kb_config.led_animation_id;
    gcr_desired = kb_config.gcr_desired;
    led_lighting_mode = kb_config.led_lighting_mode;

    bool prev_led_animation_breathing = led_animation_breathing;
    led_animation_breathing = kb_config.led_animation_breathing;
    if (led_animation_breathing && !prev_led_animation_breathing) {
        gcr_breathe = gcr_desired;
        led_animation_breathe_cur = BREATHE_MIN_STEP;
        breathe_dir = 1;
    }

    led_animation_direction = kb_config.led_animation_direction;
    led_animation_speed = kb_config.led_animation_speed;

    bool led_enabled = kb_config.led_enabled;
    I2C3733_Control_Set(led_enabled);

#ifdef CONSOLE_ENABLE
    uprintf("Loading saved settings from EEPROM:\n");
    uprintf("  led_animation_id %d\n", led_animation_id);
    uprintf("  gcr_desired %d\n", gcr_desired);
    uprintf("  led_lighting_mode %d\n", led_lighting_mode);
    uprintf("  led_animation_breathing %d\n", led_animation_breathing);
    uprintf("  led_animation_direction %d\n", led_animation_direction);
    uprintf("  led_animation_speed %f\n", led_animation_speed);
    uprintf("  led_enabled %d\n", led_enabled);
#endif
}

void save_settings(void) {
    // Save the keyboard config to EEPROM
    eeconfig_update_kb(kb_config.raw);
#ifdef CONSOLE_ENABLE
    uprintf("Saving settings to EEPROM\n");
#endif
}

void sync_settings(void) {
    save_settings();
    load_saved_settings();
}

void keyboard_post_init_kb(void) {
#ifdef CONSOLE_ENABLE
    uprintf("Running keyboard post-init\n");
#endif
    load_saved_settings();
}

void eeconfig_init_kb(void) {
#ifdef CONSOLE_ENABLE
    uprintf("Running eeconfig_init_kb\n");
#endif
    kb_config.raw = 0;
    kb_config.led_animation_id = 0;
    kb_config.led_lighting_mode = 0;
    kb_config.led_animation_breathing = false;
    kb_config.led_enabled = true;
    kb_config.led_animation_direction = 1;
    kb_config.gcr_desired = LED_GCR_MAX;
    kb_config.led_animation_speed = 4;

    save_settings();
}

void led_pattern_next(void) {
    kb_config.led_animation_id = (kb_config.led_animation_id + 1) % led_setups_count;
    sync_settings();
}

void led_pattern_prev(void) {
    kb_config.led_animation_id = (kb_config.led_animation_id - 1) % led_setups_count;
    sync_settings();
}

void led_mode_next(void) {
    kb_config.led_lighting_mode = (kb_config.led_lighting_mode + 1) % LED_MODE_MAX_INDEX;
    sync_settings();
}

void gcr_desired_increase(void) {
    int brightness = kb_config.gcr_desired + LED_GCR_STEP;
    kb_config.gcr_desired = brightness > LED_GCR_MAX ? LED_GCR_MAX : brightness;
    sync_settings();
}

void gcr_desired_decrease(void) {
    int brightness = kb_config.gcr_desired - LED_GCR_STEP;
    kb_config.gcr_desired = brightness < 0 ? 0 : brightness;
    sync_settings();
}

void led_set_enabled(bool enabled) {
    kb_config.led_enabled = enabled;
    sync_settings();
}

void led_set_animation_breathing(bool breathing) {
    kb_config.led_animation_breathing = breathing;
    sync_settings();
}

void led_animation_speed_increase(void) {
    kb_config.led_animation_speed += 1;
    sync_settings();
}

void led_animation_speed_decrease(void) {
    kb_config.led_animation_speed = kb_config.led_animation_speed < 1
        ? 0
        : kb_config.led_animation_speed - 1;
    sync_settings();
}

#define MODS_SHIFT  (get_mods() & MOD_BIT(KC_LSHIFT) || get_mods() & MOD_BIT(KC_RSHIFT))
#define MODS_CTRL  (get_mods() & MOD_BIT(KC_LCTL) || get_mods() & MOD_BIT(KC_RCTRL))
#define MODS_ALT  (get_mods() & MOD_BIT(KC_LALT) || get_mods() & MOD_BIT(KC_RALT))

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    switch (keycode) {
        case L_BRI:
            if (record->event.pressed) {
                gcr_desired_increase();
            }
            return false;
        case L_BRD:
            if (record->event.pressed) {
                gcr_desired_decrease();
            }
            return false;
        case L_PTN:
            if (record->event.pressed) {
                led_pattern_next();
            }
            return false;
        case L_PTP:
            if (record->event.pressed) {
                led_pattern_prev();
            }
            return false;
        case L_PSI:
            if (record->event.pressed) {
                led_animation_speed_increase();
            }
            return false;
        case L_PSD:
            if (record->event.pressed) {
                led_animation_speed_decrease();
            }
            return false;
        case L_T_MD:
            if (record->event.pressed) {
                led_mode_next();
            }
            return false;
        case L_T_ONF:
            if (record->event.pressed) {
                led_set_enabled(!kb_config.led_enabled);
            }
            return false;
        case L_ON:
            if (record->event.pressed) {
                led_set_enabled(true);
            }
            return false;
        case L_OFF:
            if (record->event.pressed) {
                led_set_enabled(false);
            }
            return false;
        case L_T_BR:
            if (record->event.pressed) {
                led_set_animation_breathing(!led_animation_breathing);
            }
            return false;
        case L_T_PTD:
            if (record->event.pressed) {
                led_animation_direction = !led_animation_direction;
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        default:
            return true; //Process all other keycodes normally
    }
}

