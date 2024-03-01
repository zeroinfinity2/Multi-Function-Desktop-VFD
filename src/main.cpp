#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <arduinoFFT.h>
#include <rotary_enc.h>

// Display Pins
#define CLK_PIN 8
#define DATA_PIN 10
#define CS_PIN 9
#define RESET_PIN 11
#define LIGHT_PIN A0

// Rotary Encoder Pins
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4

enum class Modes {
  CLOCK,
  MENU,
  ALARM,
  SET_CLOCK,
  SET_ALARM,
  ADJUST_BRIGHTNESS,
  ADJUST_TEMP
};

// Set the clock mode as the initial mode
Modes mode = Modes::CLOCK;
uint16_t lightLevel;
uint64_t runTime;

// Buffer update
uint32_t const updateBuffer = 1000;
uint64_t lastBufferTime = 0;
uint32_t const menuTimeout = 8000;

// Brightness Adujustment
uint32_t const updateBrightness = 5000;
uint64_t lastBrightnessTime = 0;
uint8_t brightnessLevel = 255;

// RTC Stuff
RTC_DS1307 rtc;
uint8_t dispHour = 0;
DateTime now;

// Function Prototypes
uint8_t getBrightness();
void setBrightness();
void print2digits(uint8_t number);
void updateEncoder();
void changeMode(Modes mode);
void cacheDateTime(DateTime time);
void processSetClock();
void clockModeInterrupt(uint8_t type);
void setClockInterrupt(uint8_t type);
uint8_t daysInMonth(uint8_t month);

const char* mainMenuItems[5] = {
  "Set Clock",
  "Set Alarm",
  "Adjust Brightness",
  "Adjust Temperature",
  "Close Menu"
};

uint64_t interruptTime = 0;
bool itemSelected = false;

// Second level selected timeData structure
struct timeData {
  // The item knows the state it is in.
  bool selected = false;
  // The index of the currently selected item.
  uint8_t itemIndex = 0;
  // The value of the currently selected item.
  uint8_t itemValue = 0;
  // The minimum integer value of the selected item
  uint8_t itemsMin = 0;
  // The maximum value of the selected item
  uint8_t itemsMax = 0;
};

// Declare all of the timeData structs
timeData hour;
timeData minute;
timeData timeOfDay;
timeData month;
timeData day;
timeData year;

struct timeData timeValues[6];

/* Display Constructor 
Pins are unique to setup.
Constructor Setup: rotation, clock, data, CS, DC, reset
*/
U8G2_GP1294AI_256X48_F_4W_SW_SPI Display(U8G2_R0, CLK_PIN, DATA_PIN, CS_PIN, U8X8_PIN_NONE, RESET_PIN);

// Rotary encoder constructor
RotEncoder Encoder(ENC_CLK, ENC_DT, ENC_SW);

/**************************************************************************/
/*
  A reimagined menu maker for the U8g2 library and compatable displays.
  Created for the 256 x 48 VFD Dot Matrix : GP1294AI.
  However, it should be compatable with any other displays.
  Note that it references the display. Construct the display first, before 
  the Menu Maker object.

*/
/**************************************************************************/
class Menumaker {
  public:
    // Sets up the variables for the display.
    int displayHeight = Display.getDisplayHeight();
    int displayWidth = Display.getDisplayWidth();
    int currentHighlighted;
    int centerPt = displayWidth / 2;
    int maxLength;
    int itemsPerPage;
    int pages;
    int _pageExpr;
    int currentPage;
    #define H_MENU 0
    #define V_MENU 1

    // Constructs a new menu.
    // Required is the maximum number of menu items, and horizontal or vertical.
    Menumaker(int maxLength, int flag) {
      this -> maxLength = maxLength;
      currentHighlighted = 0;

      if (flag & V_MENU) {
        itemsPerPage = (displayHeight - 11) / 11;
      }
      else {
        itemsPerPage = 3;
      }
    }

    // Resets the menu.
    void reset() {
      currentHighlighted = 0;
    };

    // Sets the title
    void setTitle(const char *title) {
      Display.drawButtonUTF8(centerPt, 8, U8G2_BTN_BW0 | U8G2_BTN_HCENTER, displayWidth, 0, 0, title);
      Display.drawHLine(0, 10, displayWidth);
    };

    // Builds the Menu items
    // List type = char
    void buildItems(int length, const char* items[]) {    
      // Determine the page to build
      currentPage = currentHighlighted / itemsPerPage;
      //_pageExpr = maxLength / itemsPerPage;
      //totalPages = _pageExpr + (((_pageExpr) + 1) % (_pageExpr));
      // Draw the elements
      int ySpacing = 20;
      for (int i = 0; i < itemsPerPage; i++) {
        int index = i + (currentPage * itemsPerPage);
        Display.drawButtonUTF8(centerPt, ySpacing, U8G2_BTN_BW0 | U8G2_BTN_HCENTER, displayWidth, 0, 1, items[index]);
        ySpacing += 11;
      }
      menuHighlighter(currentHighlighted, items);
    };

    // Builds the Menu Items
    // List type = timeData Struct
    void buildItems(int length, timeData items[]) {
      // Determine the horizontal page to build
      currentPage = currentHighlighted / itemsPerPage;

      // Draw the elements
      int xSpacing = 0;
      for (int i = 0; i< itemsPerPage; i++) {
        int index = i + (currentPage * itemsPerPage);
        Display.setCursor(xSpacing, 20);
        Display.print(items[index].itemValue);
        xSpacing += 20;
      }
      menuHighlighter(currentHighlighted);
    }

    // Highlights the current element
    void menuHighlighter(int menuIndex, const char* items[]) {
      Display.drawButtonUTF8(centerPt, ((menuIndex % itemsPerPage) * 11) + 20, U8G2_BTN_BW0 | U8G2_BTN_HCENTER | U8G2_BTN_INV, displayWidth, 0, 1, items[menuIndex]);
    };
    void menuHighlighter(int menuIndex) {
      Display.drawFrame(20, (menuIndex % itemsPerPage) * 20, 20, 20);
    };

    // Moves the index of the current highlighted item upwards.
    void scrollUp() {
      currentHighlighted -= 1;
      currentHighlighted = max(currentHighlighted, 0);
    };

    // Moves the index of the currently highlighted item downwards.
    void scrollDown() {
      currentHighlighted += 1;
      currentHighlighted = min(currentHighlighted, maxLength - 1);
    };
};

// Main Menu Constructor
Menumaker MainMenu(6, V_MENU);

// Set Clock Constructor
Menumaker SetClock(7, H_MENU);

// ----------------------------------------------------------
void setup() {
  pinMode(CLK_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(LIGHT_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Display.begin(); // Init the display
  Display.setContrast(brightnessLevel); // x = 0(low) to 255, initialize brightness

  Encoder.begin(); // Init the encoder

  // Call updateEncoder when changes are seen on the encoder pins
  // 0, 1 are pins 2, 3 respectively.
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

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

  // Setup the timeData structs
  // Hours
  hour.itemIndex = 0;
  hour.itemsMax = 12;
  hour.itemsMin = 1;
  hour.itemValue = 12;
  timeValues[0] = hour;

  // Minutes
  minute.itemIndex = 1;
  minute.itemsMax = 59;
  minute.itemsMin = 0;
  minute.itemValue = 0;
  timeValues[1] = minute;

  // Time of Day
  timeOfDay.itemIndex = 2;
  timeOfDay.itemsMax = 1;
  timeOfDay.itemsMin = 0;
  timeOfDay.itemValue = 0;
  timeValues[2] = timeOfDay;

  // Month
  month.itemIndex = 3;
  month.itemsMax = 12;
  month.itemsMin = 1;
  month.itemValue = 1;
  timeValues[3] = month;

  // Day
  day.itemIndex = 4;
  day.itemsMax = 31;
  day.itemsMin = 1;
  day.itemValue = 1;
  timeValues[4] = day;

  // Year
  year.itemIndex = 5;
  year.itemsMax = 2099;
  year.itemsMin = 2000;
  year.itemValue = 2024;
  timeValues[6] = year;
}

// ----------------------------------------------------------
void loop() {
  // Get the current running time
  runTime = millis();

  switch (mode) {
    case Modes::CLOCK:
      // Encoder button listener
      if (Encoder.selectorPressed()) changeMode(mode);
    
      // ------------------------ CLOCK MODE -------------------
      // Clock mode's buffer time is 1000ms.
      if ((runTime - lastBufferTime) > updateBuffer) {
        Display.clearBuffer();
        // Date / Time Display
        now = rtc.now();
        cacheDateTime(now);
        dispHour = now.hour();

        // 12 Hour Clock. Comment out for 24 hour clock.
        dispHour = ((now.hour() % 12) == 0) ? 12 : now.hour() % 12;
        // end 12 Hour clock

        // Print Time to the display.
        Display.setFont(u8g2_font_7_Seg_33x19_mn);
        Display.setCursor(0, 0);
        print2digits(dispHour);
        Display.print(":");

        Display.setCursor(45, 0);
        print2digits(now.minute());
        Display.print(":");

        Display.setCursor(90, 0);
        print2digits(now.second());

        // AM/PM Marker
        Display.setFont(u8g2_font_roentgen_nbp_t_all);
        if (now.hour() < 12) {
          Display.drawButtonUTF8(12, 45, U8G2_BTN_INV|U8G2_BTN_BW2|U8G2_BTN_HCENTER, 0,  2,  2, "AM" );
        } else {
          Display.drawButtonUTF8(12, 45, U8G2_BTN_INV|U8G2_BTN_BW2|U8G2_BTN_HCENTER, 0,  2,  2, "PM" );
        }

        // Light Sensor Data
        uint16_t width = static_cast<int>((getBrightness() * 30.0) / 255.0);
        Display.setFont(u8g2_font_waffle_t_all);
        Display.drawGlyph(130, 46, 57966); // icon
        Display.drawFrame(140, 40, 30, 5); 

        Display.drawBox(140, 40, width, 5);

        // Print Date to the display
        Display.setFont(u8g2_font_spleen8x16_mf);
        Display.setCursor(35, 46);
        print2digits(now.month());
        Display.print("/");
        print2digits(now.day());
        Display.print("/");
        Display.print(now.year());

        // Set a new buffer updated time
        Display.sendBuffer();
        lastBufferTime = runTime;
      } 
      break;
      // END of CLOCK MODE
    
    case Modes::MENU:
      // Encoder button listener
      if (Encoder.selectorPressed()) changeMode(mode);

      // ------------------------ MENU MODE -------------------
      Display.clearBuffer();
      // Build the menu
      Display.setFont(u8g2_font_roentgen_nbp_t_all);
      MainMenu.setTitle("Settings");
      MainMenu.buildItems(6, mainMenuItems);
      Display.sendBuffer();
      
      break;
    
    case Modes::SET_ALARM:
      // ------------------------ ALARM SET MODE -------------------
      Display.clearBuffer();
      SetClock.buildItems(6, timeValues);

      Display.sendBuffer();
      break;
    
    case Modes::ALARM:
      // ------------------------ ALARM MODE -------------------
      // todo
      break;

    case Modes::SET_CLOCK:
      // ------------------------ SET CLOCK MODE -------------------
      // The encoder button is context aware in the set clock mode
      if (Encoder.selectorPressed()) {
        // This is the switch to determine if an item is selected or not.
        itemSelected = !itemSelected;

        if (itemSelected) {
          // Sets the current item selected to the currently highlighted item.
          

        }
      }
      if ((runTime - lastBufferTime) > updateBuffer) {
        Display.clearBuffer();
        // If an item in this mode is not selected, the 

        //rtc.adjust(DateTime(year, month, day, hour, minute, second));
        // Processes the input by allowing user to adjust the values
        // of each contained value, based on index.
        // 0: 12 hour, 1: minutes, 2: Am/Pm, 3: Month, 4: day, 5: year, 6: quit

        // Adjust hour, minute, AM/PM

        // Adjust month, day, year


        Display.sendBuffer();
        
        lastBufferTime = runTime;
      }
      break;
    
    case Modes::ADJUST_BRIGHTNESS:
      // ------------------------ ADJUST BRIGHTNESS MODE -------------------
      // todo
      break;
    
    case Modes::ADJUST_TEMP:
      // ------------------------ ADJUST TEMP MODE -------------------
      // todo
      break;    
  }
  // Finally, auto adjust the brightness
  setBrightness();
}

// --------------------------------- METHODS -------------------------------------

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

// This gets called when an interrupt is detected
void updateEncoder() {
  // Gets the event type.
  uint8_t eventType = Encoder.encoderEvent();

  // Context sensitive interrupt
  if (mode == Modes::MENU) {
    clockModeInterrupt(eventType);
  }
  else if (mode == Modes::SET_CLOCK) {
    setClockInterrupt(eventType);
  }
}

// Handles the logic behind mode changes
void changeMode(Modes currentMode) {
  // Open the Main menu in Clockmode.
  if (currentMode == Modes::CLOCK) mode = Modes::MENU;

  // Change the mode based on the selected index in
  // Menu mode.
  if (currentMode == Modes::MENU) {
    switch (MainMenu.currentHighlighted) {
      case 0:
        mode = Modes::SET_CLOCK;
        break;

      case 1:
        mode = Modes::SET_ALARM;
        break;
      
      case 2:
        mode = Modes::ADJUST_BRIGHTNESS;
        break;
      
      case 3:
        mode = Modes::ADJUST_TEMP;
        break;
      
      default:
        mode = Modes::CLOCK;
        break;
    }
  }
  MainMenu.reset();
  lastBufferTime = 0;
}

// Get the light level from the photodiode from 0 - 255.
uint8_t getBrightness() {
  lightLevel = analogRead(0);
  uint8_t brightness = static_cast<int>((lightLevel / 1023.0) * 255.0);
  return brightness;
}

// Adjust the display brightness
void setBrightness() {
  if ((runTime - lastBrightnessTime) > updateBrightness) {
          brightnessLevel = getBrightness();
          Display.setContrast(brightnessLevel);  
          lastBrightnessTime = runTime;
  }
}

// Cache the Date and Time
void cacheDateTime(DateTime time) {
  // Update the structs.
  hour.itemValue = time.twelveHour();
  minute.itemValue = time.minute();
  timeOfDay.itemValue = time.isPM();
  month.itemValue = time.month();
  day.itemValue = time.day();
  year.itemValue = time.year();

}

// Processes the Set Clock selections
void processSetClock() {
}

// Mode interrupts
// These are called when the encoder is rotated in each associated mode

void clockModeInterrupt(uint8_t eventType) {
  if (eventType == CLOCKWISE) {
    MainMenu.scrollDown();
    Serial.println(mainMenuItems[MainMenu.currentHighlighted]);
  }
  else if (eventType == COUNTERCLOCKWISE) {
    MainMenu.scrollUp();
    Serial.println(mainMenuItems[MainMenu.currentHighlighted]);
  }
}
void setClockInterrupt(uint8_t eventType) {
  // If an item is selected, then the menu
  // is on a secondary level.
  // Increment or decrement the selected item's value.
  // If not, then resume normal item scrolling.
  if (eventType == CLOCKWISE) {
    if (itemSelected) {
      //increment
    }
    else SetClock.scrollDown();
  }
  else if (eventType == COUNTERCLOCKWISE) {
    if (itemSelected) {
      // decrement
    }
    else SetClock.scrollUp();
  }
}

// This function determines the number of days in the month
uint8_t daysInMonth(uint8_t month) {
  uint8_t days = 28 + (month + (month / 8)) % 2 + (2 % month + (2 * (1 / month)));
  return days;
}