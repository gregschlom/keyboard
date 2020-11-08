#include <Arduino.h>
#include <LedDisplay.h>
#include <usb_keyboard.h>
#include "keycodes.h"

LedDisplay display =  LedDisplay(/*dataPin=*/0, 
                                 /*registerSelect=*/1, 
                                 /*clockPin=*/2, 
                                 /*chipEnable=*/3, 
                                 /*resetPin=*/25, 
                                 /*displayLength=*/16);

int8_t brightness = 8;           // dislpay brightness

constexpr int8_t cols[] = {10, 7, 6, 5, 21, 20, 9, 12, 22, 16, 15, 11, 14, 18, 17};
constexpr int8_t rows[] = {19, 8, 4, 23, 13};
constexpr int8_t kNumCols = sizeof(cols);
constexpr int8_t kNumRows = sizeof(rows);

static_assert(kNumCols < 16, "cannot handle more than 16 columns");
static_assert(kNumRows < 8, "cannot handle more than 8 rows");

#define IS_MOD(code) (KC_LCTRL <= (code) && (code) <= KC_RGUI)
#define MOD_INDEX(code) ((code) & 0b111)
#define MOD_BIT(code) (1 << MOD_INDEX(code))

constexpr uint8_t layout[kNumRows][kNumCols] = {
 { KC_ESCAPE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL, KC_GRAVE, KC_PAUSE },
 { KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET, KC_BSPACE, KC_NULL },
 { KC_LCTRL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, KC_ENTER, KC_NULL, KC_NULL},
 { KC_LSHIFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, KC_RSHIFT, KC_NULL, KC_UP, KC_NULL },
 { KC_LALT, KC_LGUI, KC_NULL, KC_NULL, KC_NULL, KC_SPACE, KC_NULL, KC_NULL, KC_NULL, KC_RALT, KC_RGUI, KC_LEFT, KC_DOWN, KC_RIGHT, KC_NULL },
};

void setup() {
  display.begin();
  display.setBrightness(brightness);
  display.clear();
  
  for (int8_t i = 0; i<kNumCols; ++i) { 
    pinMode(cols[i], INPUT_PULLDOWN); 
  }
  for (int8_t i = 0; i<kNumRows; ++i) { 
    pinMode(rows[i], OUTPUT); 
    digitalWrite(rows[i], LOW);
  }
}

void loop() {
  delay(10);
  
  uint8_t key_index = 0;
  keyboard_modifier_keys = 0;
  memset(keyboard_keys, 0, sizeof(keyboard_keys));

  for (int8_t r = 0; r < kNumRows; ++r) { 
    digitalWrite(rows[r], HIGH);
    delayMicroseconds(5);
    for (int8_t c = 0; c < kNumCols; ++c) { 
      if (digitalRead(cols[c])) {
        uint8_t key = layout[r][c];
        if (IS_MOD(key)) {
          keyboard_modifier_keys |= MOD_BIT(key);
        } else {
          keyboard_keys[key_index++] = key;
          if (key_index >= 6) break;
        }
      }
    }
    digitalWrite(rows[r], LOW);
    if (key_index >= 6) break;
  }
  
  // display.home();
  // if (keyboard_keys[0] || 
  //     keyboard_keys[1] || 
  //     keyboard_keys[2] || 
  //     keyboard_keys[3] || 
  //     keyboard_keys[4] ||
  //     keyboard_keys[5] ) {
  //   display.print("  ");
  //   for (int i = 0; i < 6; ++i) {
  //     if (keyboard_keys[i] < 16) display.print("0");
  //     display.print(keyboard_keys[i], HEX);
  //   }        
  // } else {
  //   display.print("--");
  // }

  usb_keyboard_send();
}

