#ifndef __STATUS_LED_H
#define __STATUS_LED_H

#include "globals.h"

//extern Adafruit_NeoPixel  StatusLed;

// ─── Status LED ───────────────────────────────────────────────────────────────
void initStatusLED(void);
void setStatusLED(uint8_t r, uint8_t g, uint8_t b);


//set brightness
//set effect
//set pulse 

#endif //__STATUS_LED_H