#ifndef __OLED_H
#define __OLED_H

#define SCREEN_WIDTH          128
#define SCREEN_HEIGHT         32
#define OLED_RESET            4
#define SCREEN_ADDRESS        0x3C

#include <Adafruit_SSD1306.h>
#include "globals.h"
#include "settings.h"

//extern Adafruit_SSD1306   display;

bool initOledDisplay(void);

void drawWiFiIcon(int x, int y) ;

void displayStatus(const String& line1, const String& line2 = "", const String& line3 = "", const String& line4 = "", bool showWiFiIcon = false);

void displayConnectionStatus(const String& type, int remainingTime = -1);



#endif