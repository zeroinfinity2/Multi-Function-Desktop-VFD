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

RotEncoder::RotEncoder(uint8_t clockPin, uint8_t dtPin, uint8_t switchPin) {
    _clockPin = clockPin;
    _dtPin = dtPin;
    _switchPin = switchPin;
}

void RotEncoder::begin() {
    // Set the pin modes on the arduino
    pinMode(_clockPin, INPUT);
    pinMode(_dtPin, INPUT);
    pinMode(_switchPin, INPUT_PULLUP);

    // Default values
    _debounce = 500;
    _selectorPrevTime = 0;
    _clkPrevState = 0;
    _selectorState = 1;
}

void RotEncoder::readEncoder() {
    getCurrentClk(_clockPin);
    getDtState(_dtPin);
}

void RotEncoder::getCurrentClk(uint8_t clockPin) {
    _clkState = digitalRead(clockPin);
}

void RotEncoder::getDtState(uint8_t dtPin) {
    _dtState = digitalRead(dtPin);
}

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

RotEncoder::Direction RotEncoder::encoderEvent() {
    // Reads current states
    readEncoder();

    // If the clk state is 1, an event happened
    // And the previous state is not 1 (only registers one event)
    if (_clkState != _clkPrevState && _clkState == 1) {
        // Determine the direction of rotation
        // When the states differ, encoder was rotated CCW
        if (_dtState != _clkState) {
            _currentDirection = Direction::COUNTERCLOCKWISE;
        }
        // If they are the same, encoder was rotated CW
        else {
            _currentDirection = Direction::CLOCKWISE;
        }
    }
    // Set the current CLK state to the previous state.
    _clkPrevState = _clkState; 

    return _currentDirection;
}