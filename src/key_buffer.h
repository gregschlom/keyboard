#ifndef KEY_BUFFER_H
#define KEY_BUFFER_H

#include <Arduino.h>

template<int MAX_SIZE>
class KeyBuffer {
public:
  KeyBuffer() = default;

  bool push(uint8_t value) {
    if (size_ >= MAX_SIZE) return false;
    buffer_[size_++] = value;
    return true;
  }

  uint8_t pop() {
    if (size_ == 0) return 0;
    return buffer_[--size_];
  }

  bool contains(uint8_t value) {
    for (uint8_t i = 0; i < size_; ++i) {
      if (buffer_[i] == value) { 
        return true;
      }
    }
    return false;
  }

  void clear() {
    size_ = 0;
  }

private:
  uint8_t size_ = 0;
  uint8_t buffer_[MAX_SIZE] = {};
};

#endif
