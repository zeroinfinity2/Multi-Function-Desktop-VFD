/**************************************************************************/
/*!
  @file     rotary_enc.h

  Basic library for a rotary encoder, used in the VFD project.
  The encoder also utilizes a push button.
  By: Christopher Sawyer

  License: GPLv3
*/
/**************************************************************************/
#ifndef rotary_enc_h
#define rotary_enc_h

#include <Arduino.h>

// No complete step yet.
#define NEUTRAL 0x0
// Clockwise step.
#define CLOCKWISE 0x10
// Anti-clockwise step.
#define COUNTERCLOCKWISE 0x20

class RotEncoder {
    public:
      // Constructor for a new encoder
      RotEncoder(uint8_t clockPin, uint8_t dtPin, uint8_t switchPin);

      // Initializes the rotary encoder
      void begin();
      
      // Gets the current reading on the CLK pin (HIGH or LOW)
      void getCurrentClk(uint8_t _clockPin);

      // Gets the current reading from the dt pin (HIGH or LOW)
      // DT is phase shifted from the primary CLK.
      void getDtState(uint8_t _dtPin);

      // Read the selector's state, return true or false
      bool selectorPressed();

      // Records the state of the encoder
      void readEncoder();

      // Called when a change is registered on an interrupt pin
      // Returns the type of event that happened.
      uint8_t encoderEvent();

    private:
      // The pins the encoder is connected to
      uint8_t _clockPin;
      uint8_t _dtPin;
      uint8_t _switchPin;
      
      // The instantaneous state of the encoder pins
      volatile uint8_t _encState;
      
      // The state of the selector button
      volatile uint8_t _selectorState;

      // The last recorded time the selector was pressed
      volatile uint64_t _selectorPrevTime;

      // The debounce delay for the selector
      volatile uint64_t _debounce;
};
#endif