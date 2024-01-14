#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>


int lightLevel;
float brightness;
long runTime;



/* Display Constructor 
  Pins are unique to setup.
  Constructor Setup: rotation, clock, data, CS, DC, reset
*/
U8G2_GP1294AI_256X48_F_4W_SW_SPI Display(U8G2_R0, 8, 10, 9, U8X8_PIN_NONE, 11);

void setup(void) {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  Display.begin(); // Init the display
  Display.setContrast(150); // x = 0(low) to 255
  Serial.begin(9600);
}

int getBrightness() {
  lightLevel= analogRead(0);
  brightness = static_cast<int>((lightLevel / 1023.0) * 255.0);
  return brightness;
} 

void loop(void) {
  // Get the running time
  runTime = millis();

  // Only update the buffer every 2000 ms.
  if ((runTime % 2000) == 0) {
    Display.clearBuffer();

    // Draw a 256 x 28 Frame in the display
    Display.drawFrame(0, 0, 256, 48);

    Display.setFont( u8g2_font_5x7_mf);
    Display.drawStr(10,20,"Test");

    Display.setFont(u8g2_font_6x10_mf);
    Display.drawStr(40,20,"Test");

    Display.setFont(u8g2_font_roentgen_nbp_t_all);
    Display.drawStr(80,20,"Test");

    Display.setFont(u8g2_font_simple1_te);
    Display.drawStr(130,20,"Test");


    Display.setCursor(10, 40);

    // Light Sensor Data
    Display.setFont(u8g2_font_6x10_mf);
    Display.print("Light Sensor Brightness: ");
    Display.print((getBrightness() / 255.0) * 100.0);
    Display.print("%");
    Display.sendBuffer();

  }

  // Automatically adjust brightness every 5000 ms.
  if ((runTime % 5000) == 0) {
    Display.setContrast(getBrightness());  
  }

}