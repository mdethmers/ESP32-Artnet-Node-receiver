#include "globals.h"
#include "statusLed.h"
#include <Adafruit_NeoPixel.h>

#define STATU_LED_BRIGH_DIV 2


Adafruit_NeoPixel  StatusLed(1, STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);

void initStatusLED(void)
{
    StatusLed.begin();
}

// ─── Status LED ───────────────────────────────────────────────────────────────
void setStatusLED(uint8_t r, uint8_t g, uint8_t b) {
  StatusLed.setPixelColor(0, StatusLed.Color(r/STATU_LED_BRIGH_DIV, g/STATU_LED_BRIGH_DIV, b/STATU_LED_BRIGH_DIV));
  StatusLed.show();
}