#include "outputsDriver.h"
#include "globals.h"

#include <I2SClocklessLedDriver.h>   // WS2812 driver
#include <FastLED.h>                 // needed by I2SAPA102
#include <I2SAPA102.h>               // APA102 driver
#include <Adafruit_NeoPixel.h>

#include "settings.h"
#include "oled.h"


const int DEFAULT_PINS[GPIO_OUTPUTS_NBR]  = {GPIO_STRIPS_OUTPUT_1,GPIO_STRIPS_OUTPUT_2,GPIO_STRIPS_OUTPUT_3,GPIO_STRIPS_OUTPUT_4};


// WS2812 driver + raw byte buffer
// MAX_LEDS_RGB is the largest possible count (RGB mode), so size the buffer for that.
I2SClocklessLedDriver driver;
uint8_t ledData[GPIO_OUTPUTS_NBR * MAX_LEDS_RGB * 5]; // worst-case: 5 bytes per LED (RGBWW)

// Grouped output writes an expanded frame instead of handing the network buffer
// straight to the driver, and showPixels(NO_WAIT) keeps reading that frame while
// the next one arrives. Two buffers, used alternately, keep the transfer in
// progress away from the frame being built. The second one is only allocated
// when grouping is actually switched on; ungrouped nodes pay nothing for it.
// If the allocation fails we fall back to one buffer and a blocking show, which
// is slower but never tears.
uint8_t    *ledFrames[2]   = { ledData, ledData };
uint8_t     ledFrameIdx    = 0;
DisplayMode groupShowMode  = NO_WAIT;

// APA102 driver + CRGB buffer
I2SAPA102 controller(0);
CRGB      leds[GPIO_OUTPUTS_NBR * MAX_LEDS_RGB];      // FastLED CRGB array for APA102


// NeoPixel strip objects (used for static/test colour output only)
  Adafruit_NeoPixel strips[GPIO_OUTPUTS_NBR] = {
 /* Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[0], NEO_BRG + NEO_KHZ800),
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[1], NEO_BRG + NEO_KHZ800),*/
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[2], NEO_BRG + NEO_KHZ800),
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[3], NEO_BRG + NEO_KHZ800)
};



// ─── Update strip pins ────────────────────────────────────────────────────────
void updateStripPins(void) 
{
  for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) {
    if (strips[i].getPin() != settings.device.pins[i]) {
      strips[i].setPin(settings.device.pins[i]);
      strips[i].begin();
    }
  }
}

void reconfigureNeoPixelStrips(void)
{
  // Reconfigure NeoPixel strip objects for static/test colour use
  uint32_t pixelType;
  switch (settings.fullFrameMode.channelMode) {
    case CHANNEL_MODE_RGBWW: pixelType = NEO_GRBW + NEO_KHZ800; break; // 5-ch not natively in Adafruit; RGBW used for basic test
    case CHANNEL_MODE_RGBW:  pixelType = NEO_GRBW + NEO_KHZ800; break;
    default:                 pixelType = NEO_BRG  + NEO_KHZ800; break;
  }
  for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) {
    strips[i].updateType(pixelType);
    strips[i].updateLength(settings.fullFrameMode.numLedsPerOutput);
  }
}

void initNeoPixelStrips(void)
{
  // Blank all strips at startup
  for (int i = 0; i < settings.device.numOutputs; i++) {
    strips[i].begin();
    for (int j = 0; j < settings.fullFrameMode.numLedsPerOutput; j++) strips[i].setPixelColor(j, 0);
    strips[i].show();
  }
}


void initOutputDriver(void)
{
// ── Initialise the correct LED driver ────────────────────────────────────
  // The library requires uint8_t* for the pins array; settings.pins is int[].
  // Copy into a local uint8_t array before every driver init call.
  uint8_t activePins[GPIO_OUTPUTS_NBR];
  for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) activePins[i] = (uint8_t)settings.device.pins[i];

  if (settings.device.ledType == LED_TYPE_APA102) {
    displayStatus("Initializing...", "APA102 driver");
    Serial.println("LED type: APA102");
    int activePinsInt[GPIO_OUTPUTS_NBR];
    for (int i = 0; i < GPIO_OUTPUTS_NBR; i++) activePinsInt[i] = settings.device.pins[i];
    controller.initled(leds, activePinsInt, APA102_CLOCK_PIN,
                       settings.device.numOutputs, settings.fullFrameMode.numLedsPerOutput);
    controller.setBrightness(settings.fullFrameMode.ledBrightness);
    // Quick white flash to confirm wiring
    fill_solid(leds, settings.device.numOutputs * settings.fullFrameMode.numLedsPerOutput, CRGB::White);
    controller.showPixels();
    delay(200);
    fill_solid(leds, settings.device.numOutputs * settings.fullFrameMode.numLedsPerOutput, CRGB::Black);
    controller.showPixels();

  } else {
    // WS2812 family ─────────────────────────────────────────────────────────
    // Correct overload: initled(uint8_t* leds, uint8_t* pins,
    //                           uint8_t numStrips, uint16_t numLedPerStrip,
    //                           ColorArrangement cArr)
    switch (settings.fullFrameMode.channelMode) {

      case CHANNEL_MODE_RGBWW:
        displayStatus("Initializing...", "WS2812 RGBWW (5ch)");
        Serial.println("LED type: WS2812 RGBWW/CCT (5ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)GPIO_OUTPUTS_NBR, (uint16_t)settings.fullFrameMode.numLedsPerOutput,
                       ORDER_RGBCCT);
        driver.setBrightness(settings.fullFrameMode.ledBrightness);
        break;

      case CHANNEL_MODE_RGBW:
        displayStatus("Initializing...", "WS2812 RGBW (4ch)");
        Serial.println("LED type: WS2812 RGBW (4ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)GPIO_OUTPUTS_NBR, (uint16_t)settings.fullFrameMode.numLedsPerOutput,
                       ORDER_GRBW);
        driver.setBrightness(settings.fullFrameMode.ledBrightness);
        break;

      default:
        displayStatus("Initializing...", "WS2812 RGB (3ch)");
        Serial.println("LED type: WS2812 RGB (3ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)GPIO_OUTPUTS_NBR, (uint16_t)settings.fullFrameMode.numLedsPerOutput,
                       ORDER_GRB);
        driver.setBrightness(settings.fullFrameMode.ledBrightness);
        break;
    }
    // Clear the buffer after driver init — prevents stale bytes causing
    // a rogue full-white first LED when ArtNet data starts arriving.
    memset(ledData, 0, sizeof(ledData));

    // Grouped output builds each frame itself, so it needs somewhere to build
    // the next one while the driver is still clocking out the last. Claimed
    // before the Art-Net buffers, which take whatever heap is left over.
    if (getGroupSize() > 1) {
      uint8_t *second = (uint8_t *)calloc(sizeof(ledData), 1);
      if (second) {
        ledFrames[1]  = second;
        groupShowMode = NO_WAIT;
        Serial.printf("Grouping x%d — double-buffered\n", getGroupSize());
      } else {
        // One buffer left: block until the frame is on the wire before the next
        // one overwrites it. Costs the callback task the transmit time.
        groupShowMode = WAIT;
        Serial.println("Grouping — second frame buffer unavailable, using blocking show");
      }
    }
  }
}