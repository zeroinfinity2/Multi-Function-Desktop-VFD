#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

/* Display Constructor 
  Pins are unique to setup.
  Constructor Setup: rotation, clock, data, CS, DC, reset
*/

U8G2_GP1294AI_256X48_1_4W_SW_SPI Display(U8G2_R0, 13, 12, 10, U8X8_PIN_NONE, 8);

void setup(void) {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(8, OUTPUT);
  Display.begin(); // Init the display
  Display.setContrast(150); // x = 0(low) to 255
}

// draw something on the display with the `firstPage()`/`nextPage()` loop
int test = 0;

void loop(void) {
  Display.firstPage();
  do {
    Display.setFont(u8g2_font_luBS12_te);
    Display.drawStr(0,20,"This is a test message");
    Display.setCursor(20, 40);
    Display.print(test);

  } while (Display.nextPage() );
  delay(1000);
  test = (test + 1) % 100;
}