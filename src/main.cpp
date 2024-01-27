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

enum Modes {
  CLOCK,
  MENU,
  ALARM,
  SET_CLOCK,
  SET_ALARM,
  SET_DATE,
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

// Function Prototypes
uint8_t getBrightness();
void setBrightness();
void print2digits(uint8_t number);
void updateEncoder();
void changeMode(Modes mode);

const char* mainMenuItems[6] = {
  "Set Clock",
  "Set Alarm",
  "Set Date",
  "Adjust Brightness",
  "Adjust Temperature",
  "Close Menu"
};

bool menuLoaded = false;
uint64_t interruptTime = 0;

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
    int currentSelected;
    int centerPt = displayWidth / 2;
    int maxLength;
    int itemsPerPage;
    int pages;
    int _pageExpr;
    int currentPage;

    // Constructs a new menu.
    // Required is the maximum number of menu items
    Menumaker(int maxLength) {
      this -> maxLength = maxLength;
      currentSelected = 0;
      itemsPerPage = (displayHeight - 11) / 11;
    }

    // Resets the menu.
    void reset() {
      currentSelected = 0;
    };

    // Sets the title
    void setTitle(const char *title) {
      Display.drawButtonUTF8(centerPt, 8, U8G2_BTN_BW0 | U8G2_BTN_HCENTER, displayWidth, 0, 0, title);
      Display.drawHLine(0, 10, displayWidth);
    };

    // Builds the Menu items
    void buildItems(int length, const char* items[]) {
      
      // Determine the page to build
      currentPage = currentSelected / itemsPerPage;
      //_pageExpr = maxLength / itemsPerPage;
      //totalPages = _pageExpr + (((_pageExpr) + 1) % (_pageExpr));


      // Draw the elements
      int ySpacing = 20;
      for (int i = 0; i < itemsPerPage; i++) {
        int index = i + (currentPage * itemsPerPage);
        Display.drawButtonUTF8(centerPt, ySpacing, U8G2_BTN_BW0 | U8G2_BTN_HCENTER, displayWidth, 0, 1, items[index]);
        ySpacing += 11;
      }

      menuHighlighter(currentSelected, items);
    };

    // Highlights the currently selected element
    void menuHighlighter(int menuIndex, const char* items[]) {
      Display.drawButtonUTF8(centerPt, ((menuIndex % itemsPerPage) * 11) + 20, U8G2_BTN_BW0 | U8G2_BTN_HCENTER | U8G2_BTN_INV, displayWidth, 0, 1, items[menuIndex]);
    };

    // Moves the index of the current selected item upwards.
    void scrollUp() {
      currentSelected -= 1;
      currentSelected = max(currentSelected, 0);
    };

    // Moves the index of the currently selected item downwards.
    void scrollDown() {
      currentSelected += 1;
      currentSelected = min(currentSelected, maxLength - 1);
    };
};

// Main Menu Constructor
Menumaker MainMenu(6);

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
}

// ----------------------------------------------------------
void loop() {
  // Get the current running time
  runTime = millis();
  
  // Defines the logic of pressing the selector.
  if (Encoder.selectorPressed()) {
    changeMode(mode);
  }

  switch (mode) {
    case Modes::CLOCK:
      // ------------------------ CLOCK MODE -------------------
      // Clock mode's buffer time is 1000ms.
      if ((runTime - lastBufferTime) > updateBuffer) {
        Display.clearBuffer();
        // Date / Time Display
        DateTime now = rtc.now();
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
      // todo
      if ((runTime - lastBufferTime) > updateBuffer) {
        Display.clearBuffer();

        Display.sendBuffer();
        lastBufferTime = runTime;
      }
      break;
    
    case Modes::ALARM:
      // ------------------------ ALARM MODE -------------------
      // todo
      break;

    case Modes::SET_CLOCK:
      // ------------------------ SET CLOCK MODE -------------------
      // todo
      break;

    case Modes::SET_DATE:
      // ------------------------ SET DATE MODE -------------------
      // todo
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
  // If the menu mode is active
  if (mode == Modes::MENU) {
    uint8_t eventType = Encoder.encoderEvent();

    if (eventType == CLOCKWISE) {
      MainMenu.scrollDown();
      Serial.println(mainMenuItems[MainMenu.currentSelected]);
    }
    else if (eventType == COUNTERCLOCKWISE) {
      MainMenu.scrollUp();
      Serial.println(mainMenuItems[MainMenu.currentSelected]);
    }
  }
}

// Handles the logic behind mode changes
void changeMode(Modes currentMode) {
  // Open the Main menu in Clockmode.
  if (currentMode == Modes::CLOCK) mode = Modes::MENU;

  // Change the mode based on the selected index in
  // Menu mode.
  if (currentMode == Modes::MENU) {
    switch (MainMenu.currentSelected) {
      case 0:
        mode = Modes::SET_CLOCK;
        break;

      case 1:
        mode = Modes::SET_ALARM;
        break;
      
      case 2:
        mode = Modes::SET_DATE;
        break;
      
      case 3:
        mode = Modes::ADJUST_BRIGHTNESS;
        break;
      
      case 4:
        mode = Modes::ADJUST_TEMP;
        break;
      
      default:
        mode = Modes::CLOCK;
        break;
    }
    
  }

  MainMenu.reset();
  menuLoaded = false;
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
