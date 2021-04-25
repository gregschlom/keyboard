#include <Arduino.h>
#include <LedDisplay.h>
#include <usb_keyboard.h>

#include "key_buffer.h"
#include "keycodes.h"

LedDisplay display = LedDisplay(/*dataPin=*/0,
                                /*registerSelect=*/1,
                                /*clockPin=*/2,
                                /*chipEnable=*/3,
                                /*resetPin=*/25,
                                /*blankPin=*/24,
                                /*displayLength=*/16);

int8_t brightness = 10;  // display brightness

constexpr int8_t cols[] = {10, 7,  6,  5,  21, 20, 9, 12,
                           22, 16, 15, 11, 14, 18, 17};
constexpr int8_t rows[] = {19, 8, 4, 23, 13};
constexpr int8_t kNumCols = sizeof(cols);
constexpr int8_t kNumRows = sizeof(rows);
constexpr int8_t kSecKeyPin = 27;

static_assert(kNumCols < 16, "cannot handle more than 16 columns");
static_assert(kNumRows < 8, "cannot handle more than 8 rows");

#define IS_MOD(code) (KC_LCTRL <= (code) && (code) <= KC_RGUI)
#define MOD_INDEX(code) ((code)&0b111)
#define MOD_BIT(code) (1 << MOD_INDEX(code))

// clang-format off
constexpr uint8_t layout[kNumRows][kNumCols] = {
    {KC_ESCAPE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL, KC_GRAVE, KC_BSLASH},
    {KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET, KC_BSPACE, KC_NULL},
    {KC_LCTRL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, KC_ENTER, KC_NULL, KC_NULL},
    {KC_LSHIFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, KC_RSHIFT, KC_NULL, KC_UP, KC_NULL},
    {KC_LALT, KC_LGUI, KC_NULL, KC_NULL, KC_NULL, KC_SPACE, KC_NULL, KC_NULL, KC_NULL, KC_RGUI, KC_FN, KC_LEFT, KC_DOWN, KC_RIGHT, KC_NULL},
};
// clang-format on

bool menu_mode = false;
bool wait_release = false;
bool win_mode = true;
bool fn_pressed = false;

// Timestamp of the last time any key was pressed.
// Used to put the display to sleep
uint32_t last_interaction = 0;

void processReplacements(KeyBuffer<6> &pressed_keys, uint8_t &modifiers) {
  if (win_mode) {
    bool alt_pressed = modifiers & MOD_BIT(KC_LALT);
    bool gui_pressed = modifiers & MOD_BIT(KC_LGUI);
    // Flip alt and gui if only one of them is pressed
    if (alt_pressed != gui_pressed) {
      modifiers ^= MOD_BIT(KC_LALT);
      modifiers ^= MOD_BIT(KC_LGUI);
    }
  }

  if (fn_pressed) {  // modifiers & MOD_BIT(KC_RALT)
    // enter/exit menu mode
    if (pressed_keys.process_single(KC_BSLASH)) {
      if (!wait_release) {
        menu_mode = !menu_mode;
        wait_release = true;
      }
    } else if (wait_release) {
      wait_release = false;
    }

    // Simulate delete
    if (pressed_keys.process_single(KC_BSPACE)) {
      pressed_keys.push(KC_DELETE);
    }

    if (pressed_keys.process_single(KC_ESCAPE)) {
      display.clear();
      display.print("Key");
      display.flush();
      pinMode(kSecKeyPin, INPUT_PULLDOWN);
      delay(100);
      pinMode(kSecKeyPin, INPUT_DISABLE);
      display.clear();
      display.flush();
    }
  }
}

void setup() {
  display.begin();
  display.setBrightness(brightness);
  display.clear();
  display.home();
  display.print("Hello");

  for (int8_t i = 0; i < kNumCols; ++i) {
    pinMode(cols[i], INPUT_PULLDOWN);
  }
  for (int8_t i = 0; i < kNumRows; ++i) {
    pinMode(rows[i], OUTPUT);
    digitalWrite(rows[i], LOW);
  }

  last_interaction = millis();

  pinMode(kSecKeyPin, INPUT_DISABLE);
}

void loop() {
  delay(10);

  if (millis() - last_interaction > 60 * 1000) {
    display.clear();
  } else {
    display.home();
    if (menu_mode) {
      display.print("Menu ");
    } else {
      display.print(win_mode ? "Win   " : "Mac   ");
    }
  }
  display.flush();

  fn_pressed = false;
  uint8_t modifiers = 0;
  KeyBuffer<6> pressed_keys;

  for (int8_t r = 0; r < kNumRows; ++r) {
    digitalWrite(rows[r], HIGH);
    delayMicroseconds(5);
    for (int8_t c = 0; c < kNumCols; ++c) {
      if (digitalRead(cols[c])) {
        last_interaction = millis();
        uint8_t key = layout[r][c];
        if (key == KC_FN) {
          fn_pressed = true;
        } else if (IS_MOD(key)) {
          modifiers |= MOD_BIT(key);
        } else {
          pressed_keys.push(key);
        }
      }
    }
    digitalWrite(rows[r], LOW);
  }

  processReplacements(pressed_keys, modifiers);

  if (menu_mode) {
    usb_keyboard_release_all();

    uint8_t key = pressed_keys.pop();
    bool shift = modifiers & (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT));
    char c = '\0';
    pressed_keys.clear();
    if (key >= KC_A && key <= KC_Z) {
      c = (shift ? 'A' : 'a') + key - KC_A;
    }
    if (key != 0 || modifiers != 0) {
      display.print(c ? c : key);
      display.print('/');
      display.print(modifiers);
      display.print("/     ");
      display.flush();
    }
    if (c == 'W') {
      win_mode = true;
      menu_mode = false;
      display.clear();
    }
    if (c == 'M') {
      win_mode = false;
      menu_mode = false;
      display.clear();
    }
    return;
  }

  keyboard_modifier_keys = modifiers;
  for (int i = 0; i < 6; ++i) {
    keyboard_keys[i] = pressed_keys.pop();
  }
  usb_keyboard_send();
}
