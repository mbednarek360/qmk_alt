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

keymap_config_t keymap_config;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME, \
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGUP, \
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN, \
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, MO(1),   KC_LEFT, KC_DOWN, KC_RGHT  \
    ),
    [1] = LAYOUT(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, KC_MUTE, \
        L_T_BR,  L_PSD,   L_BRI,   L_PSI,   _______, _______, _______, _______, U_T_AGCR,_______, KC_PSCR, KC_SLCK, KC_PAUS, _______, KC_END, \
        L_T_PTD, L_PTP,   L_BRD,   L_PTN,   _______, _______, _______, _______, _______, _______, _______, _______,          _______, KC_VOLU, \
        _______, L_T_MD,  L_T_ONF, _______, _______, MD_BOOT, NK_TOGG, _______, _______, _______, _______, _______,          KC_PGUP, KC_VOLD, \
        _______, _______, _______,                            _______,                            _______, _______, KC_HOME, KC_PGDN, KC_END  \
    ),
    /*
    [X] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, _______  \
    ),
    */
};

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
};

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
};

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

led_instruction_t led_instructions[] = {
    //LEDs are normally inactive, no processing is performed on them
    //Flags are used in matching criteria for an LED to be active and indicate how to color it
    //Flags can be found in tmk_core/protocol/arm_atsam/led_matrix.h (prefixed with LED_FLAG_)
    //LED IDs can be found in config_led.h in the keyboard's directory
    //Examples are below

    //All LEDs use the user's selected pattern (this is the factory default)
     { .flags = LED_FLAG_USE_ROTATE_PATTERN },

    //Specific LEDs use the user's selected pattern while all others are off
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_ROTATE_PATTERN, .id0 = 0xFFFFFFFF, .id1 = 0xAAAAAAAA, .id2 = 0x55555555, .id3 = 0x11111111 },

    //Specific LEDs use specified RGB values while all others are off
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 0xFF, .id1 = 0x00FF, .id2 = 0x0000FF00, .id3 = 0xFF000000, .r = 75, .g = 150, .b = 225 },

    //All LEDs use the user's selected pattern
    //On layer 1, all key LEDs (except the top row which keeps active pattern) are red while all edge LEDs are green
    //When layer 1 is active, key LEDs use red    (id0  32 -  17: 1111 1111 1111 1111 0000 0000 0000 0000 = 0xFFFF0000) (except top row 16 - 1)
    //When layer 1 is active, key LEDs use red    (id1  64 -  33: 1111 1111 1111 1111 1111 1111 1111 1111 = 0xFFFFFFFF)
    //When layer 1 is active, key LEDs use red    (id2  87 -  65: 0000 0000 0111 1111 1111 1111 1111 1111 = 0x007FFFFF)
    //When layer 1 is active, edge LEDs use green (id2  95 -  88: 1111 1111 1000 0000 0000 0000 0000 0000 = 0xFF800000)
    //When layer 1 is active, edge LEDs use green (id3 119 -  96: 0000 0000 1111 1111 1111 1111 1111 1111 = 0x00FFFFFF)
    // { .flags = LED_FLAG_USE_ROTATE_PATTERN },
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_RGB, .id0 = 0xFFFF0000, .id1 = 0xFFFFFFFF, .id2 = 0x007FFFFF, .r = 255, .layer = 1 },
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_MATCH_LAYER | LED_FLAG_USE_RGB, .id2 = 0xFF800000, .id3 = 0x00FFFFFF, .g = 127, .layer = 1 },

    //All key LEDs use red while edge LEDs use the active pattern
    //All key LEDs use red     (id0  32 -   1: 1111 1111 1111 1111 1111 1111 1111 1111 = 0xFFFFFFFF)
    //All key LEDs use red     (id1  64 -  33: 1111 1111 1111 1111 1111 1111 1111 1111 = 0xFFFFFFFF)
    //All key LEDs use red     (id2  87 -  65: 0000 0000 0111 1111 1111 1111 1111 1111 = 0x007FFFFF)
    //Edge uses active pattern (id2  95 -  88: 1111 1111 1000 0000 0000 0000 0000 0000 = 0xFF800000)
    //Edge uses active pattern (id3 119 -  96: 0000 0000 1111 1111 1111 1111 1111 1111 = 0x00FFFFFF)
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_RGB, .id0 = 0xFFFFFFFF, .id1 = 0xFFFFFFFF, .id2 = 0x007FFFFF, .r = 255 },
    // { .flags = LED_FLAG_MATCH_ID | LED_FLAG_USE_ROTATE_PATTERN , .id2 = 0xFF800000, .id3 = 0x00FFFFFF },

    //end must be set to 1 to indicate end of instruction set
     { .end = 1 }
};
