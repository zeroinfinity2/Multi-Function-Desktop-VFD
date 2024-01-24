/**************************************************************************/
/*!
  @file     menumaker.h
  A reimagined menu maker for the U8g2 library and compatable displays.
  Created for the 256 x 48 VFD Dot Matrix : GP1294AI.
  However, it should be compatable with any other displays.

  Requires: U8G2 Library
  By: Christopher Sawyer

  License: GPLv3
*/
/**************************************************************************/
#ifndef menumaker_h
#define menumaker_h

#include <Arduino.h>
#include <U8g2lib.h>

class Menumaker {
    public:
        
      /*  The constructor for the Menu Maker object. Note that it requires 
          a reference to the display. Construct the display first, before 
          the Menu Maker object.

          For now, change the constructor to your display.
      */
      Menumaker(U8G2_GP1294AI_256X48_F_4W_SW_SPI Display);

      U8G2_GP1294AI_256X48_F_4W_SW_SPI _display;
      
      int displayHeight;

      int displayWidth;

      void begin();

      
};
#endif