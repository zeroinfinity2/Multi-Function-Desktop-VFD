#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RTClib.h>

uint16_t lightLevel;
uint64_t runTime;

// Buffer update
uint32_t const updateBuffer = 1000;
uint64_t lastBufferTime = 0;

// Brightness Adujustment
uint32_t const updateBrightness = 5000;
uint64_t lastBrightnessTime = 0;

// RTC Stuff
RTC_DS1307 rtc;
uint8_t dispHour = 0;
String timeDay;

// Function Prototypes
uint8_t getBrightness();
void print2digits(uint8_t number);

/* Display Constructor 
Pins are unique to setup.
Constructor Setup: rotation, clock, data, CS, DC, reset
*/
U8G2_GP1294AI_256X48_F_4W_SW_SPI Display(U8G2_R0, 8, 10, 9, U8X8_PIN_NONE, 11);


// ----------------------------------------------------------
void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(A0, INPUT_PULLUP);

  Serial.begin(9600);
  Display.begin(); // Init the display
  Display.setContrast(5); // x = 0(low) to 255, initialize brightness

  // Find the RTC Module
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// ----------------------------------------------------------
void loop() {
  // Get the current running time
  runTime = millis();

  // Only update the buffer every updateBuffer amount.
  if ((runTime - lastBufferTime) > updateBuffer) {
    Display.clearBuffer();

    // Draw a 255 x 48 Frame in the display
    Display.drawFrame(0, 0, 255, 48);

    Display.setFont( u8g2_font_5x7_mf);
    Display.drawStr(10,20,"Test");

    Display.setFont(u8g2_font_6x10_mf);
    Display.drawStr(40,20,"Test");

    Display.setFont(u8g2_font_roentgen_nbp_t_all);
    Display.drawStr(80,20,"Test");

    Display.setFont(u8g2_font_simple1_te);
    Display.drawStr(130,20,"Test");

    // Light Sensor Data
    Display.setCursor(10, 40);
    Display.setFont(u8g2_font_6x10_mf);
    Display.print("Light Sensor Brightness: ");
    Display.print((getBrightness() / 255.0) * 100.0);
    Display.print("%");

    // Date / Time Display
    DateTime now = rtc.now();
    dispHour = now.hour();

    // 12 Hour Clock. Comment out for 24 hour clock.
    dispHour = ((now.hour() % 12) == 0) ? 12 : now.hour() % 12;
    timeDay = ((now.isPM() == 1)) ? "PM" : "AM";
    // end 12 Hour clock

    // Print Time to the display.
    Display.setFont(u8g2_font_roentgen_nbp_t_all);
    Display.setCursor(155, 10);
    print2digits(dispHour);
    Display.print(":");
    print2digits(now.minute());
    Display.print(":");
    print2digits(now.second());
    Display.print(timeDay);

    // Print Date to the display
    Display.setCursor(155, 20);
    print2digits(now.month());
    Display.print("/");
    print2digits(now.day());
    Display.print("/");
    Display.print(now.year());

    // Automatically adjust brightness by updateBrightness setting.
    if ((runTime - lastBrightnessTime) > updateBrightness) {
      Display.setContrast(getBrightness());  
      lastBrightnessTime = runTime;
    }

    // Final thing to do
    // Set a new buffer updated time
    Display.sendBuffer();
    lastBufferTime = runTime;
    }
  }

// Get the light level from the photodiode and set a brightness from 0 - 255.
uint8_t getBrightness() {
  lightLevel = analogRead(0);
  uint8_t brightness = static_cast<int>((lightLevel / 1023.0) * 255.0);
  brightness = constrain(brightness, 5, 204);
  return brightness;
} 

// Prefixes single digit date and time numbers with a zero.
void print2digits(uint8_t number) {
  if (number >= 0 && number < 10) {
    Display.print("0");
    Display.print(number);
  }
  else {
    Display.print(number);
  }
}