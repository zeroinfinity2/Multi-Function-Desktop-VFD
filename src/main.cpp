#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
int lightLevel;
long runTime;

// Buffer update
const long updateBuffer = 1000;
long lastBufferTime = 0;

// Brightness Adujustment
const long updateBrightness = 5000;
long lastBrightnessTime = 0;

// RTC Stuff
tmElements_t tm;
bool parse = false;
bool config = false;
int dispHour = 0;
String timeDay;

// Function Prototypes
bool getTime(const char *str);
bool getDate(const char *str);
int getBrightness();
void print2digits(int number);



/* Display Constructor 
  Pins are unique to setup.
  Constructor Setup: rotation, clock, data, CS, DC, reset
*/
U8G2_GP1294AI_256X48_F_4W_SW_SPI Display(U8G2_R0, 8, 10, 9, U8X8_PIN_NONE, 11);

void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(A0, INPUT_PULLUP);

  Display.begin(); // Init the display
  Display.setContrast(5); // x = 0(low) to 255, initialize brightness

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
      parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }

  Serial.begin(9600);
   if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }
}

void loop() {
  // Get the current running time
  runTime = millis();

  // Only update the buffer every updateBuffer amount.
  if ((runTime - lastBufferTime) > updateBuffer) {
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

    // Light Sensor Data
    Display.setCursor(10, 40);
    Display.setFont(u8g2_font_6x10_mf);
    Display.print("Light Sensor Brightness: ");
    Display.print((getBrightness() / 255.0) * 100.0);
    Display.print("%");

    // Date / Time Display
    if(RTC.read(tm)) {
      dispHour = tm.Hour;
      
      // 12 Hour Clock. Comment out for 24 hour clock.
      dispHour = ((tm.Hour % 12) == 0) ? 12 : tm.Hour % 12;
      timeDay = ((tm.Hour < 12) && (tm.Hour >= 0)) ? "AM" : "PM";
      // end 12 Hour clock

      // Print Time to the display.
      Display.setFont(u8g2_font_roentgen_nbp_t_all);
      Display.setCursor(155, 10);
      print2digits(dispHour);
      Display.print(":");
      print2digits(tm.Minute);
      Display.print(":");
      print2digits(tm.Second);
      Display.print(timeDay);

      // Print Date to the display
      Display.setCursor(155, 20);
      print2digits(tm.Month);
      Display.print("/");
      print2digits(tm.Day);
      Display.print("/");
      Display.print(tmYearToCalendar(tm.Year));

    } 

    else { // Something's wrong with the RTC
      if (RTC.chipPresent()) {
        Serial.println("The DS1307 is stopped.");
        Serial.println();
        } 
      else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
      }
    }

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

// Sets the time to the time manager.
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

// Sets the date to the time manager.
bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

// Get the light level from the photodiode and set a brightness from 0 - 255.
int getBrightness() {
  lightLevel = analogRead(0);
  int brightness = static_cast<int>((lightLevel / 1023.0) * 255.0);
  brightness = constrain(brightness, 5, 204);
  return brightness;
} 

// Prefixes single digit date and time numbers with a zero.
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Display.print("0");
    Display.print(number);
  }
  else {
    Display.print(number);
  }
}