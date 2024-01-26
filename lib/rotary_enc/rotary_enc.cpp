/**************************************************************************/
/*!
  @file     rotary_enc.h

  Basic library for a rotary encoder, used in the VFD project.
  The encoder also utilizes a push button.
  By: Christopher Sawyer

  License: GPLv3
*/
/**************************************************************************/
#include <rotary_enc.h>
#include <Arduino.h>

#define R_START 0x0
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | CLOCKWISE},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | COUNTERCLOCKWISE},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

RotEncoder::RotEncoder(uint8_t clockPin, uint8_t dtPin, uint8_t switchPin) {
    _clockPin = clockPin;
    _dtPin = dtPin;
    _switchPin = switchPin;
    _debounce = 500;
}

void RotEncoder::begin() {
    // Set the pin modes on the arduino
    pinMode(_clockPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    pinMode(_switchPin, INPUT_PULLUP);

    // Default values
    _selectorPrevTime = 0;
}

uint8_t RotEncoder::encoderEvent() {
    // Grab state of input pins.
    uint8_t pinstate = (digitalRead(_dtPin) << 1) | digitalRead(_clockPin);
    // Determine new state from the pins and state table.
    _encState = ttable[_encState & 0xf][pinstate];
    // Return emit bits, ie the generated event.
    return _encState & 0x30;
};

bool RotEncoder::selectorPressed() {
    _selectorState = digitalRead(_switchPin);
    if ((_selectorState == 0) && (millis() - _selectorPrevTime > _debounce)) {
        _selectorPrevTime = millis();
        return true;
    }
    else {
        return false;
    }
}