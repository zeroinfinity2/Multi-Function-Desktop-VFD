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

class RotEncoder {
    public:

      // Rotations of the encoder
      enum class Direction {
        NEUTRAL = 0,
        CLOCKWISE = 1,
        COUNTERCLOCKWISE = -1
      };

      // Constructor for a new encoder
      RotEncoder(uint8_t clockPin, uint8_t dtPin, uint8_t switchPin);
      
      // Gets the current reading on the CLK pin (HIGH or LOW)
      void getCurrentClk(uint8_t _clockPin);

      // Gets the current reading from the dt pin (HIGH or LOW)
      // DT is phase shifted from the primary CLK.
      void getDtState(uint8_t _dtPin);

      // Gets the last direction the encoder was rotated
      Direction getDirection();

      // Sets the direction of rotation of the encoder
      void setDirection();

      // Read the selector's state, return true or false
      bool selectorPressed();

      // Records the state of the encoder
      void readEncoder();

      // Called when a change is registered on an interrupt pin
      void encoderEvent();

    private:
      // The pins the encoder is connected to
      uint8_t _clockPin;
      uint8_t _dtPin;
      uint8_t _switchPin;

      // The direction the encoder has been rotated
      Direction _currentDirection;
      
      // The instantaneous state of the CLK pin
      volatile uint8_t _clkState;

      // The instantaneous state of the DT pin
      volatile uint8_t _dtState;

      // The last reported state of the CLK pin.
      volatile uint8_t _clkPrevState;
      
      // The state of the selector button
      volatile uint8_t _selectorState;

      // The last recorded time the selector was pressed
      volatile uint64_t _selectorPrevTime;

      // The debounce delay for the selector
      volatile uint64_t _debounce;
};
#endif