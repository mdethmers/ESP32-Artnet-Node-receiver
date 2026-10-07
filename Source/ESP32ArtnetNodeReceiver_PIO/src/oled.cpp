#include "globals.h"
#include <Adafruit_SSD1306.h>
#include "oled.h"


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool initOledDisplay(void)
{
  bool err = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if(!err) {    Serial.println(F("SSD1306 allocation failed"));  }
  return err;
}

// ─── OLED helpers ─────────────────────────────────────────────────────────────
void drawWiFiIcon(int x, int y) {
  display.drawPixel(x,     y+5, SSD1306_WHITE);
  display.drawPixel(x,     y+6, SSD1306_WHITE);
  display.drawPixel(x,     y+7, SSD1306_WHITE);
  display.drawPixel(x+2,   y+3, SSD1306_WHITE);
  display.drawPixel(x+2,   y+4, SSD1306_WHITE);
  display.drawPixel(x+2,   y+5, SSD1306_WHITE);
  display.drawPixel(x+2,   y+6, SSD1306_WHITE);
  display.drawPixel(x+2,   y+7, SSD1306_WHITE);
  display.drawPixel(x+4,   y+1, SSD1306_WHITE);
  display.drawPixel(x+4,   y+2, SSD1306_WHITE);
  display.drawPixel(x+4,   y+3, SSD1306_WHITE);
  display.drawPixel(x+4,   y+4, SSD1306_WHITE);
  display.drawPixel(x+4,   y+5, SSD1306_WHITE);
  display.drawPixel(x+4,   y+6, SSD1306_WHITE);
  display.drawPixel(x+4,   y+7, SSD1306_WHITE);
}

void displayStatus(const String& line1, const String& line2, const String& line3,
                   const String& line4, bool showWiFiIcon) {
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  if(settings.device.screenRotation)   display.setRotation(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);  display.println(line1);
  if (line2.length()) { display.setCursor(0,  9); display.println(line2); }
  if (line3.length()) { display.setCursor(0, 18); display.println(line3); }
  if (line4.length()) { display.setCursor(0, 27); display.println(line4); }
  if (showWiFiIcon)   drawWiFiIcon(SCREEN_WIDTH - 8, SCREEN_HEIGHT - 10);
  display.display();
}

void displayConnectionStatus(const String& type, int remainingTime) {
  String line2 = (remainingTime >= 0) ? "Timeout in: " + String(remainingTime) + "s" : "";
  displayStatus("Waiting for " + type, line2);
}