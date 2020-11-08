#include <Arduino.h>
#include <LedDisplay.h>
#include <usb_keyboard.h>

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

#define KEY_TRNS 0

enum Mods : uint8_t {
    KC_LCTRL = 0xE0,
    KC_LSHIFT,
    KC_LALT,
    KC_LGUI,
    KC_RCTRL,
    KC_RSHIFT,
    KC_RALT,
    KC_RGUI
};

#define IS_MOD(code) (KC_LCTRL <= (code) && (code) <= KC_RGUI)
#define MOD_INDEX(code) ((code) & 0b111)
#define MOD_BIT(code) (1 << MOD_INDEX(code))

constexpr uint8_t layout[kNumRows][kNumCols] = {
 { KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_TILDE, KEY_PAUSE },
 { KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACE, KEY_RIGHT_BRACE, KEY_BACKSPACE, KEY_TRNS },
 { KC_LCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_ENTER, KEY_TRNS, KEY_TRNS},
 { KC_LSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KC_RSHIFT, KEY_TRNS, KEY_UP, KEY_TRNS },
 { KC_LALT, KC_LGUI, KEY_TRNS, KEY_TRNS, KEY_TRNS, KEY_SPACE, KEY_TRNS, KEY_TRNS, KEY_TRNS, KC_RALT, KC_RGUI, KEY_LEFT, KEY_DOWN, KEY_RIGHT, KEY_TRNS },
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

