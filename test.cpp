#include <Arduino.h>
char DateAndTimeString[20]; //19 digits plus the null char
int thisYear = 2016;
int thisMonth = 1;
int thisDay = 11;
int thisHour = 9;
int thisMinute = 9;
int thisSecond = 30;

void setup()
{
    Serial.begin(115200);
    sprintf_P(DateAndTimeString, PSTR("%4d-%02d-%02d %d:%02d:%02d"), thisYear, thisMonth, thisDay, thisHour, thisMinute, thisSecond);
    Serial.println(DateAndTimeString);
}

void loop() {}