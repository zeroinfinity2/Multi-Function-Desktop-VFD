/**************************************************************************/
/*!
  @file     menumaker.cpp
  A reimagined menu maker for the U8g2 library and compatable displays.
  Created for the 256 x 48 VFD Dot Matrix : GP1294AI.
  However, it should be compatable with any other displays.

  Requires: U8G2 Library
  By: Christopher Sawyer

  License: GPLv3
*/
/**************************************************************************/
#include <Arduino.h>
#include <U8g2lib.h>
#include <menumaker.h>

Menumaker::Menumaker(U8G2_GP1294AI_256X48_F_4W_SW_SPI Display) : _display(Display) {

};

void Menumaker::begin() {
    Serial.println(_display.getDisplayHeight());
};
