#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

/* Display Constructor 
  Pins are unique to setup.
  Constructor Setup: rotation, clock, data, CS, DC, reset
*/

U8G2_GP1294AI_256X48_1_4W_SW_SPI Display(U8G2_R0, 13, 12, 10, U8X8_PIN_NONE, 8);

void setup(void) {
  Display.begin(); // Init the display
}

// draw something on the display with the `firstPage()`/`nextPage()` loop
void loop(void) {
  Display.firstPage();
  do {
    Display.setFont(u8g2_font_ncenB14_tr);
    Display.drawStr(0,20,"Hello World!");
  } while (Display.nextPage() );
  delay(1000);
}