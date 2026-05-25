/**
 * ArtNet LED Controller for ESP32
 * Supports WS2812 (I2SClocklessLedDriver) AND APA102 (I2SAPA102).
 * Switch between them live via the web UI — a save+restart applies the change.
 *
 * All other features are preserved:
 *   - Ethernet / WiFi / AP / RGB-test / Static-colour modes
 *   - OLED status display, NeoPixel status LED, boot-button mode cycling
 *   - OTA firmware update page
 *   - Persistent settings in NVS (Preferences)
 *
 * Channel modes: 0 = RGB (3ch), 1 = RGBW (4ch), 2 = RGBWW/CCT (5ch)
 *
 * NEW: Random Fade mode — smoothly fades between random colours on all
 *      active channels (respects RGB / RGBW / RGBWW mode).
 *      Enable via web UI "Colour Options" section.
 */

// ─── Feature flags ────────────────────────────────────────────────────────────
#define DEBUG_ETHERNET_WEBSERVER_PORT Serial
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 1
#define UNIQUE_SUBARTNET

// ─── LED configuration ────────────────────────────────────────────────────────
#define MAX_LEDS_RGB          680   // 170 LEDs × 4 universes  (3 ch)
#define MAX_LEDS_RGBW         512   // 128 LEDs × 4 universes  (4 ch)
#define MAX_LEDS_RGBWW        408   // 102 LEDs × 4 universes  (5 ch)  floor(512/5)*4
#define NUM_LEDS_PER_STRIP    680
#define NUMSTRIPS             4
#define UNIVERSE_SIZE_IN_CHANNEL 512
#define COLOR_GRB

// APA102 clock pin — MUST be > 16 on ESP32
#define APA102_CLOCK_PIN      17

// ─── Hardware configuration ───────────────────────────────────────────────────
#define STATUS_LED_PIN        16
#define SCREEN_WIDTH          128
#define SCREEN_HEIGHT         32
#define OLED_RESET            4
#define SCREEN_ADDRESS        0x3C
#define BOOT_BUTTON_PIN       0

int ShifterPin = 25;

// ─── Libraries ────────────────────────────────────────────────────────────────
#include <Preferences.h>
#include <WebServer_ESP32_W5500.h>
#include <WebServer.h>
#include "I2SClocklessLedDriver.h"   // WS2812 driver
#include "FastLED.h"                 // needed by I2SAPA102
#include "I2SAPA102.h"               // APA102 driver
#include "Arduino.h"
#include "artnetESP32V2.h"
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Update.h>
#include <WiFi.h>

// ─── LED type enum ────────────────────────────────────────────────────────────
enum LedType {
  LED_TYPE_WS2812 = 0,
  LED_TYPE_APA102 = 1
};

// ─── Channel mode enum ────────────────────────────────────────────────────────
enum ChannelMode {
  CHANNEL_MODE_RGB   = 0,   // 3 channels — WS2812 or APA102
  CHANNEL_MODE_RGBW  = 1,   // 4 channels — WS2812 only
  CHANNEL_MODE_RGBWW = 2    // 5 channels — WS2812 only (RGBCCT / RGBWW)
};

// ─── Default pins ─────────────────────────────────────────────────────────────
const int DEFAULT_PINS[NUMSTRIPS] = {12, 14, 27, 26};

// ─── Forward declarations ─────────────────────────────────────────────────────
void setupWiFi();
void setupEthernet();
void startAPMode();
void checkBootButton();
void handleColorSelection();
void cycleLEDs();
void randomFadeLEDs();          // NEW
void setStatusLED(uint8_t r, uint8_t g, uint8_t b);
void applyStaticColor();
void loadSettings();
void saveSettings();
void handleRoot();
void handleConfig();
void resetToDefaultPins();
void updateStripPins();
void displayStatus(const String& line1, const String& line2 = "", const String& line3 = "", const String& line4 = "", bool showWiFiIcon = false);
void displayConnectionStatus(const String& type, int remainingTime = -1);
void drawWiFiIcon(int x, int y);
void handleOTAUpdate();
void handleDoUpdate();
void handleOTAUpdateResponse();
void ESP32_W5500_onEvent();
void showLeds();
void setAllLeds(uint8_t r, uint8_t g, uint8_t b, uint8_t w1 = 0, uint8_t w2 = 0);

// ─── Settings ─────────────────────────────────────────────────────────────────
struct Settings {
  int     numLedsPerOutput  = 680;
  int     numOutputs        = 4;
  int     startUniverse     = 0;
  int     pins[NUMSTRIPS]   = {12, 14, 27, 26};
  int     ledBrightness     = 255;
  String  nodeName          = "Artnet Node ESP32";
  String  ssid              = "";
  String  password          = "";
  bool    useWiFi           = false;
  String  staticColor       = "FF8000";
  bool    enStatColor       = false;
  bool    enableCustomPins  = false;
  bool    ledCycleEnabled   = false;
  bool    randomFadeEnabled = false;   // NEW: smooth random colour fade
  bool    useStaticIP       = false;
  String  staticIP          = "192.168.1.100";
  String  gateway           = "192.168.1.1";
  String  subnet            = "255.255.255.0";
  int     networkMode       = 0;   // 0=Ethernet 1=WiFi 2=AP 3=RGB-test 4=Static
  int     channelMode       = CHANNEL_MODE_RGB; // 0=RGB, 1=RGBW, 2=RGBWW
  int     whiteLevel        = 0;
  int     white2Level       = 0;   // second white channel for RGBWW static colour
  int     ledType           = LED_TYPE_WS2812; // 0=WS2812, 1=APA102
  int     protocolMode       = PROTOCOL_BOTH;  // 0=ArtNet, 1=sACN, 2=Both

  // Convenience helpers -------------------------------------------------------
  bool isRGBW()   const { return channelMode == CHANNEL_MODE_RGBW;  }
  bool isRGBWW()  const { return channelMode == CHANNEL_MODE_RGBWW; }
  // Deprecated accessor kept so existing call-sites still compile
  bool useRGBW_compat() const { return channelMode != CHANNEL_MODE_RGB; }
};

// ─── Runtime state ────────────────────────────────────────────────────────────
struct State {
  bool          isAPMode            = false;
  bool          ledCycleEnabled     = false;
  bool          isStaticColorSet    = false;
  uint8_t       currentColor        = 0;
  unsigned long lastUpdate          = 0;
  unsigned long startAttemptTime    = 0;
  bool          ledState            = false;
  bool          colorSelectionMode  = false;
  int           selectedColorIndex  = 0;
  unsigned long lastBlinkTime       = 0;
  bool          displayBlink        = false;
};

// ─── Colour-fade state ────────────────────────────────────────────────────────
// Cycles sequentially: Red → Green → Blue → White1 (RGBW/WW) → White2 (RGBWW)
// Each channel fades 0→255 then 255→0 before the next channel takes over.
// FADE_STEP controls speed: units per tick at FADE_TICK_MS intervals.
#define FADE_STEP 5   // 1 = slow/smooth (~5 s per colour at 50 Hz); raise for faster

struct FadeState {
  uint8_t channelIndex = 0;  // outgoing channel (0=R,1=G,2=B,3=W1,4=W2)
  int     value        = 0;  // crossfade progress: brightness of INCOMING channel (0-255)
  int     direction    = 1;  // unused, kept for struct stability
  bool    initialised  = false;
} fadeState;

// ─── Preset colours ───────────────────────────────────────────────────────────
const int    PRESET_COLORS_COUNT = 10;
const String presetColors[PRESET_COLORS_COUNT] = {
  "#FF0000","#00FF00","#0000FF","#FFFF00",
  "#FF00FF","#00FFFF","#FFFFFF","#FF8000",
  "#8000FF","#000000"
};

const unsigned long CYCLE_DELAY = 1000;
// Fade tick interval — lower = smoother/faster update rate
const unsigned long FADE_TICK_MS = 20;  // ~50 Hz

// ─── Global objects ───────────────────────────────────────────────────────────
Adafruit_SSD1306   display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel  StatusLed(1, STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer          server(80);
Preferences        preferences;
artnetESP32V2      artnet = artnetESP32V2();

// WS2812 driver + raw byte buffer
// MAX_LEDS_RGB is the largest possible count (RGB mode), so size the buffer for that.
I2SClocklessLedDriver driver;
uint8_t ledData[NUMSTRIPS * MAX_LEDS_RGB * 5]; // worst-case: 5 bytes per LED (RGBWW)

// APA102 driver + CRGB buffer
I2SAPA102 controller(0);
CRGB      leds[NUMSTRIPS * MAX_LEDS_RGB];      // FastLED CRGB array for APA102

Settings settings;
State    state;

// NeoPixel strip objects (used for static/test colour output only)
Adafruit_NeoPixel strips[NUMSTRIPS] = {
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[0], NEO_BRG + NEO_KHZ800),
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[1], NEO_BRG + NEO_KHZ800),
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[2], NEO_BRG + NEO_KHZ800),
  Adafruit_NeoPixel(MAX_LEDS_RGB, DEFAULT_PINS[3], NEO_BRG + NEO_KHZ800)
};

// ─── Helper: show whatever driver is active ───────────────────────────────────
void showLeds() {
  if (settings.ledType == LED_TYPE_APA102) {
    controller.showPixels();
  } else {
    // WS2812: artnet callback calls driver.showPixels() directly.
    // This helper is only used by cycleLEDs / applyStaticColor.
  }
}

// ─── Helper: set every pixel in both buffers to one colour ───────────────────
void setAllLeds(uint8_t r, uint8_t g, uint8_t b, uint8_t w1, uint8_t w2) {
  if (settings.ledType == LED_TYPE_APA102) {
    int total = settings.numOutputs * settings.numLedsPerOutput;
    for (int i = 0; i < total; i++) leds[i] = CRGB(r, g, b);
    controller.showPixels();
  } else {
    // WS2812 family: handle RGB, RGBW and RGBWW test outputs
    if (settings.channelMode == CHANNEL_MODE_RGBWW) {
      int total = settings.numOutputs * settings.numLedsPerOutput;
      memset(ledData, 0, sizeof(ledData));
      for (int i = 0; i < total; i++) {
        ledData[i * 5 + 0] = r;
        ledData[i * 5 + 1] = g;
        ledData[i * 5 + 2] = b;
        ledData[i * 5 + 3] = w1;
        ledData[i * 5 + 4] = w2;
      }
      driver.showPixels(NO_WAIT, ledData);

    } else if (settings.channelMode == CHANNEL_MODE_RGBW) {
      for (int i = 0; i < settings.numOutputs; i++) {
        for (int j = 0; j < settings.numLedsPerOutput; j++) {
          strips[i].setPixelColor(j, r, g, b, w1);
        }
        strips[i].show();
      }

    } else {
      for (int i = 0; i < settings.numOutputs; i++) {
        for (int j = 0; j < settings.numLedsPerOutput; j++) {
          strips[i].setPixelColor(j, r, g, b);
        }
        strips[i].show();
      }
    }
  }
}

// ─── ArtNet callback ──────────────────────────────────────────────────────────
void displayfunction(void *param) {
  subArtnet *subartnet = (subArtnet *)param;

  if (settings.ledType == LED_TYPE_APA102) {
    // APA102 is always RGB — unpack into CRGB array
    uint8_t *src = subartnet->data;
    int total = settings.numOutputs * settings.numLedsPerOutput;
    for (int i = 0; i < total; i++) {
      leds[i].r = src[i * 3 + 0];
      leds[i].g = src[i * 3 + 1];
      leds[i].b = src[i * 3 + 2];
    }
    controller.showPixels();
  } else {
    // WS2812 — pass raw buffer directly (works for RGB, RGBW and RGBWW
    // because the driver was initialised with the correct colour arrangement)
    driver.showPixels(NO_WAIT, subartnet->data);
  }
}

// ─── Helper accessors ─────────────────────────────────────────────────────────
int getMaxLedsPerStrip() {
  switch (settings.channelMode) {
    case CHANNEL_MODE_RGBWW: return MAX_LEDS_RGBWW;
    case CHANNEL_MODE_RGBW:  return MAX_LEDS_RGBW;
    default:                 return MAX_LEDS_RGB;
  }
}

int getChannelsPerLed() {
  switch (settings.channelMode) {
    case CHANNEL_MODE_RGBWW: return 5;
    case CHANNEL_MODE_RGBW:  return 4;
    default:                 return 3;
  }
}

int getLedsPerUniverse() {
  // How many LEDs fit in one 512-byte ArtNet universe
  return UNIVERSE_SIZE_IN_CHANNEL / getChannelsPerLed();
}

// ─── Sequential colour fade: tick called from loop() ─────────────────────────
// Crossfades between channels with no black gap.
// While the incoming channel rises 0->255, the outgoing channel falls 255->0.
// Order: R -> G -> B -> W1 (RGBW/WW only) -> W2 (RGBWW only), then repeats.
// Adjust FADE_STEP at the top of the file to change speed.
void randomFadeLEDs() {
  static unsigned long lastTick = 0;
  unsigned long now = millis();
  if (now - lastTick < FADE_TICK_MS) return;
  lastTick = now;

  int numChannels = getChannelsPerLed(); // 3, 4, or 5

  if (!fadeState.initialised) {
    fadeState.channelIndex = 0;
    fadeState.value        = 255;  // start: current channel already at full
    fadeState.direction    = 1;    // direction now means: crossfade in progress
    fadeState.initialised  = true;
  }

  // 'value' is the brightness of the INCOMING (next) channel (0->255).
  // The OUTGOING (current) channel is 255 - value.
  fadeState.value += FADE_STEP;
  if (fadeState.value >= 255) {
    fadeState.value        = 255;
    // Crossfade complete — incoming becomes the new current channel
    fadeState.channelIndex = (fadeState.channelIndex + 1) % numChannels;
    fadeState.value        = 0;   // next crossfade starts fresh
  }

  int cur  = fadeState.channelIndex;
  int next = (cur + 1) % numChannels;
  uint8_t vIn  = (uint8_t)fadeState.value;         // incoming rises
  uint8_t vOut = (uint8_t)(255 - fadeState.value);  // outgoing falls

  uint8_t ch[5] = {0, 0, 0, 0, 0};
  ch[cur]  = vOut;
  ch[next] = vIn;

  setStatusLED(ch[0], ch[1], ch[2]);
  setAllLeds(ch[0], ch[1], ch[2], ch[3], ch[4]);
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(ShifterPin, OUTPUT);
  digitalWrite(ShifterPin, LOW);

  StatusLed.begin();
  setStatusLED(255, 255, 0);   // Yellow = initialising

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  displayStatus("Initializing...");

  loadSettings();

  // Enforce LED count limits
  int maxLeds = getMaxLedsPerStrip();
  if (settings.numLedsPerOutput > maxLeds) {
    settings.numLedsPerOutput = maxLeds;
    saveSettings();
  }

  // Reconfigure NeoPixel strip objects for static/test colour use
  uint32_t pixelType;
  switch (settings.channelMode) {
    case CHANNEL_MODE_RGBWW: pixelType = NEO_GRBW + NEO_KHZ800; break; // 5-ch not natively in Adafruit; RGBW used for basic test
    case CHANNEL_MODE_RGBW:  pixelType = NEO_GRBW + NEO_KHZ800; break;
    default:                 pixelType = NEO_BRG  + NEO_KHZ800; break;
  }
  for (int i = 0; i < NUMSTRIPS; i++) {
    strips[i].updateType(pixelType);
    strips[i].updateLength(settings.numLedsPerOutput);
  }
  updateStripPins();

  // Blank all strips at startup
  for (int i = 0; i < settings.numOutputs; i++) {
    strips[i].begin();
    for (int j = 0; j < settings.numLedsPerOutput; j++) strips[i].setPixelColor(j, 0);
    strips[i].show();
  }

// ── Initialise the correct LED driver ────────────────────────────────────
  // The library requires uint8_t* for the pins array; settings.pins is int[].
  // Copy into a local uint8_t array before every driver init call.
  uint8_t activePins[NUMSTRIPS];
  for (int i = 0; i < NUMSTRIPS; i++) activePins[i] = (uint8_t)settings.pins[i];

  if (settings.ledType == LED_TYPE_APA102) {
    displayStatus("Initializing...", "APA102 driver");
    Serial.println("LED type: APA102");
    int activePinsInt[NUMSTRIPS];
    for (int i = 0; i < NUMSTRIPS; i++) activePinsInt[i] = settings.pins[i];
    controller.initled(leds, activePinsInt, APA102_CLOCK_PIN,
                       settings.numOutputs, settings.numLedsPerOutput);
    controller.setBrightness(settings.ledBrightness);
    // Quick white flash to confirm wiring
    fill_solid(leds, settings.numOutputs * settings.numLedsPerOutput, CRGB::White);
    controller.showPixels();
    delay(200);
    fill_solid(leds, settings.numOutputs * settings.numLedsPerOutput, CRGB::Black);
    controller.showPixels();

  } else {
    // WS2812 family ─────────────────────────────────────────────────────────
    // Correct overload: initled(uint8_t* leds, uint8_t* pins,
    //                           uint8_t numStrips, uint16_t numLedPerStrip,
    //                           ColorArrangement cArr)
    switch (settings.channelMode) {

      case CHANNEL_MODE_RGBWW:
        displayStatus("Initializing...", "WS2812 RGBWW (5ch)");
        Serial.println("LED type: WS2812 RGBWW/CCT (5ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)NUMSTRIPS, (uint16_t)settings.numLedsPerOutput,
                       ORDER_RGBCCT);
        driver.setBrightness(settings.ledBrightness);
        break;

      case CHANNEL_MODE_RGBW:
        displayStatus("Initializing...", "WS2812 RGBW (4ch)");
        Serial.println("LED type: WS2812 RGBW (4ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)NUMSTRIPS, (uint16_t)settings.numLedsPerOutput,
                       ORDER_GRBW);
        driver.setBrightness(settings.ledBrightness);
        break;

      default:
        displayStatus("Initializing...", "WS2812 RGB (3ch)");
        Serial.println("LED type: WS2812 RGB (3ch)");
        driver.initled((uint8_t*)ledData, activePins,
                       (uint8_t)NUMSTRIPS, (uint16_t)settings.numLedsPerOutput,
                       ORDER_GRB);
        driver.setBrightness(settings.ledBrightness);
        break;
    }
    // Clear the buffer after driver init — prevents stale bytes causing
    // a rogue full-white first LED when ArtNet data starts arriving.
    memset(ledData, 0, sizeof(ledData));
  }

  // ── Networking ────────────────────────────────────────────────────────────
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  state.startAttemptTime = millis();

  switch (settings.networkMode) {
    case 0: setupEthernet(); break;
    case 1: setupWiFi();     break;
    case 2: startAPMode();   break;
    case 3:
      displayStatus("RGB Test Mode", "Cycling colors...", "Press button to exit");
      return;
    case 4:
      displayStatus("Static Color Mode", "Color: " + settings.staticColor, "Press button to exit");
      return;
    default: setupEthernet(); break;
  }

  // ── ArtNet ────────────────────────────────────────────────────────────────
  if (!state.isAPMode) {
    IPAddress activeIP = (settings.networkMode == 1) ? WiFi.localIP() : ETH.localIP();

    String ipStr = activeIP.toString();
    int lpuv = getLedsPerUniverse();
    int universesPerOutput = (settings.numLedsPerOutput + lpuv - 1) / lpuv;
    String protoStr = (settings.protocolMode == PROTOCOL_ARTNET) ? "AN" :
                    (settings.protocolMode == PROTOCOL_SACN)   ? "sACN" : "AN+sACN";
    String ledsInfo = "LED:" + String(settings.numLedsPerOutput) +
                      " Uni:" + String(universesPerOutput) +
                      " " + protoStr;
    displayStatus(settings.nodeName, ipStr, ledsInfo);

    int universeSize   = lpuv * getChannelsPerLed();
    int channelsPerLed = getChannelsPerLed();

    artnet.addSubArtnet(
      settings.startUniverse,
      settings.numLedsPerOutput * settings.numOutputs * channelsPerLed,
      universeSize,
      &displayfunction
    );
    artnet.setNodeName(settings.nodeName);

    artnet.protocolMode = settings.protocolMode;

  // Port 6454 is only used when ArtNet is active; sACN always uses 5568
  // internally. We pass 6454 as the ArtNet port regardless — the library
  // ignores it when protocolMode == PROTOCOL_SACN.
  bool listenOk = artnet.listen(activeIP, 6454);

  if (listenOk) {
    switch (settings.protocolMode) {
      case PROTOCOL_ARTNET: Serial.println("Listening — ArtNet port 6454");          break;
      case PROTOCOL_SACN:   Serial.println("Listening — sACN (E1.31) port 5568");    break;
      default:              Serial.println("Listening — ArtNet 6454 + sACN 5568");   break;
    }
  } else {
    Serial.println("ERROR: artnet.listen() failed");
  }

    // Optional: join sACN multicast groups (needed for multicast sACN sources
    // such as some lighting consoles). Unicast sACN works without this.
    if (settings.protocolMode != PROTOCOL_ARTNET) {
      int lpuv              = getLedsPerUniverse();
      int universesPerOutput = (settings.numLedsPerOutput + lpuv - 1) / lpuv;
      artnet.joinSACNMulticast(settings.startUniverse,
                               universesPerOutput * settings.numOutputs);
    }
  }
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
  checkBootButton();

  // Standalone LED modes (no network)
  if (settings.networkMode == 3 || settings.networkMode == 4) {
    if (settings.networkMode == 3 && settings.ledCycleEnabled) {
      if (settings.randomFadeEnabled) randomFadeLEDs();
      else cycleLEDs();
    } else if (settings.networkMode == 4) {
      if (state.colorSelectionMode) {
        handleColorSelection();
      } else {
        if (settings.enStatColor && !state.isStaticColorSet) {
          applyStaticColor();
          state.isStaticColorSet = true;
        }
      }
    }
    return;
  }

  server.handleClient();

  if (!state.isAPMode) {
    static bool wasDisconnected   = true;
    static bool lastConnectionState = false;

    if (settings.networkMode == 1) {
      bool networkDisconnected = (WiFi.status() != WL_CONNECTED);
      bool currentlyConnected  = !networkDisconnected;

      if (networkDisconnected) {
        static unsigned long lastBlinkTime = 0;
        if (!wasDisconnected) { Serial.println("WiFi disconnected"); wasDisconnected = true; }
        if (millis() - lastBlinkTime >= 500) {
          lastBlinkTime = millis();
          state.ledState = !state.ledState;
          setStatusLED(state.ledState ? 255 : 0, 0, 0);
          displayStatus("Waiting for Wi-Fi");
        }
      } else {
        if (wasDisconnected || !lastConnectionState) {
          setStatusLED(0, 255, 0);
          int lpuv = getLedsPerUniverse();
          int universesPerOutput = (settings.numLedsPerOutput + lpuv - 1) / lpuv;
          String ledsInfo = "LED:" + String(settings.numLedsPerOutput) +
                            " Uni:" + String(universesPerOutput) +
                            " Out:" + String(settings.numOutputs);
          displayStatus(settings.nodeName, "WiFi: " + WiFi.localIP().toString(), ledsInfo, "", true);
          wasDisconnected = false;
        }
      }
      lastConnectionState = currentlyConnected;

    } else if (settings.networkMode == 0) {
      bool networkDisconnected = (ETH.localIP() == INADDR_NONE);
      bool currentlyConnected  = !networkDisconnected;

      if (networkDisconnected) {
        static unsigned long lastBlinkTime = 0;
        if (!wasDisconnected) { Serial.println("Ethernet disconnected"); wasDisconnected = true; }
        if (millis() - lastBlinkTime >= 500) {
          lastBlinkTime = millis();
          state.ledState = !state.ledState;
          setStatusLED(state.ledState ? 255 : 0, 0, 0);
          displayStatus("Waiting for Ethernet");
        }
      } else {
        if (wasDisconnected || !lastConnectionState) {
          setStatusLED(0, 255, 0);
          String ipInfo = settings.useStaticIP ? "Static: " + settings.staticIP
                                                : "DHCP: " + ETH.localIP().toString();
          int lpuv = getLedsPerUniverse();
          int universesPerOutput = (settings.numLedsPerOutput + lpuv - 1) / lpuv;
          String ledsInfo = "LED:" + String(settings.numLedsPerOutput) +
                            " Uni:" + String(universesPerOutput) +
                            " Out:" + String(settings.numOutputs);
          displayStatus(settings.nodeName, ipInfo, ledsInfo);
          wasDisconnected = false;
        }
      }
      lastConnectionState = currentlyConnected;
    }
  }

  // ── LED output priority: randomFade > staticColor > ledCycle ─────────────
  if (settings.randomFadeEnabled) {
    randomFadeLEDs();
    state.isStaticColorSet = false;
  } else if (settings.enStatColor && !state.isStaticColorSet) {
    applyStaticColor();
    state.isStaticColorSet = true;
  } else if (settings.ledCycleEnabled) {
    cycleLEDs();
    state.isStaticColorSet = false;
  } else if (!settings.enStatColor) {
    state.isStaticColorSet = false;
  }
}

// ─── Update strip pins ────────────────────────────────────────────────────────
void updateStripPins() {
  for (int i = 0; i < NUMSTRIPS; i++) {
    if (strips[i].getPin() != settings.pins[i]) {
      strips[i].setPin(settings.pins[i]);
      strips[i].begin();
    }
  }
}

// ─── Load / save settings ─────────────────────────────────────────────────────
void loadSettings() {
  preferences.begin("esp32config", true);
  settings.numLedsPerOutput  = preferences.getInt("numledsoutput",   settings.numLedsPerOutput);
  settings.numOutputs        = preferences.getInt("numoutput",       settings.numOutputs);
  settings.startUniverse     = preferences.getInt("startuniverse",   settings.startUniverse);
  settings.nodeName          = preferences.getString("nodename",     settings.nodeName);
  settings.ledBrightness     = preferences.getInt("ledbrightness",  settings.ledBrightness);
  settings.ssid              = preferences.getString("ssid",         settings.ssid);
  settings.password          = preferences.getString("password",     settings.password);
  settings.useWiFi           = preferences.getBool("useWiFi",        settings.useWiFi);
  settings.staticColor       = preferences.getString("staticColor",  settings.staticColor);
  settings.enStatColor       = preferences.getBool("enStatColor",    settings.enStatColor);
  settings.enableCustomPins  = preferences.getBool("enCustomPins",   false);
  settings.ledCycleEnabled   = preferences.getBool("ledCycleEnabled",false);
  settings.randomFadeEnabled = preferences.getBool("randFadeEn",     false);   // NEW
  settings.useStaticIP       = preferences.getBool("useStaticIP",    settings.useStaticIP);
  settings.staticIP          = preferences.getString("staticIP",     settings.staticIP);
  settings.gateway           = preferences.getString("gateway",      settings.gateway);
  settings.subnet            = preferences.getString("subnet",       settings.subnet);
  settings.networkMode       = preferences.getInt("networkMode",     -1);
  settings.whiteLevel        = preferences.getInt("whiteLevel",      settings.whiteLevel);
  settings.white2Level       = preferences.getInt("white2Level",     settings.white2Level);
  settings.ledType           = preferences.getInt("ledType",         settings.ledType);
  settings.protocolMode      = preferences.getInt("protocolMode",    PROTOCOL_BOTH);


  // ── channelMode migration ─────────────────────────────────────────────────
  if (preferences.isKey("channelMode")) {
    settings.channelMode = preferences.getInt("channelMode", CHANNEL_MODE_RGB);
  } else {
    bool legacyRGBW = preferences.getBool("useRGBW", false);
    settings.channelMode = legacyRGBW ? CHANNEL_MODE_RGBW : CHANNEL_MODE_RGB;
  }

  if (settings.networkMode == -1)
    settings.networkMode = settings.useWiFi ? 1 : 0;

  for (int i = 0; i < NUMSTRIPS; i++) {
    char key[10]; sprintf(key, "pin%d", i);
    settings.pins[i] = preferences.getInt(key, DEFAULT_PINS[i]);
  }
  if (!settings.enableCustomPins) {
    for (int i = 0; i < NUMSTRIPS; i++) settings.pins[i] = DEFAULT_PINS[i];
  }
  preferences.end();
}

void saveSettings() {
  preferences.begin("esp32config", false);
  preferences.putInt("numledsoutput",   settings.numLedsPerOutput);
  preferences.putInt("numoutput",       settings.numOutputs);
  preferences.putInt("startuniverse",   settings.startUniverse);
  preferences.putString("nodename",     settings.nodeName);
  preferences.putInt("ledbrightness",   settings.ledBrightness);
  preferences.putString("ssid",         settings.ssid);
  preferences.putString("password",     settings.password);
  preferences.putBool("useWiFi",        settings.useWiFi);
  preferences.putString("staticColor",  settings.staticColor);
  preferences.putBool("enStatColor",    settings.enStatColor);
  preferences.putBool("enCustomPins",   settings.enableCustomPins);
  preferences.putBool("ledCycleEnabled",settings.ledCycleEnabled);
  preferences.putBool("randFadeEn",     settings.randomFadeEnabled);   // NEW
  preferences.putBool("useStaticIP",    settings.useStaticIP);
  preferences.putString("staticIP",     settings.staticIP);
  preferences.putString("gateway",      settings.gateway);
  preferences.putString("subnet",       settings.subnet);
  preferences.putInt("networkMode",     settings.networkMode);
  preferences.putInt("channelMode",     settings.channelMode);
  preferences.putInt("whiteLevel",      settings.whiteLevel);
  preferences.putInt("white2Level",     settings.white2Level);
  preferences.putInt("ledType",         settings.ledType);
  preferences.putInt("protocolMode",    settings.protocolMode);

  for (int i = 0; i < NUMSTRIPS; i++) {
    char key[10]; sprintf(key, "pin%d", i);
    preferences.putInt(key, settings.pins[i]);
  }
  preferences.end();
}

void resetToDefaultPins() {
  for (int i = 0; i < NUMSTRIPS; i++) settings.pins[i] = DEFAULT_PINS[i];
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
  display.setTextSize(1);
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

// ─── Status LED ───────────────────────────────────────────────────────────────
void setStatusLED(uint8_t r, uint8_t g, uint8_t b) {
  StatusLed.setPixelColor(0, StatusLed.Color(r/2, g/2, b/2));
  StatusLed.show();
}

// ─── Colour cycle (test mode) ─────────────────────────────────────────────────
void cycleLEDs() {
  if (millis() - state.lastUpdate < CYCLE_DELAY) return;
  state.lastUpdate = millis();

  int channels = getChannelsPerLed();
  uint8_t r = 0, g = 0, b = 0, w1 = 0, w2 = 0;
  switch (state.currentColor) {
    case 0: r  = settings.ledBrightness; break;
    case 1: g  = settings.ledBrightness; break;
    case 2: b  = settings.ledBrightness; break;
    case 3: if (channels >= 4) w1 = settings.ledBrightness; break;
    case 4: if (channels >= 5) w2 = settings.ledBrightness; break;
  }
  state.currentColor = (state.currentColor + 1) % channels;
  setAllLeds(r, g, b, w1, w2);
}

// ─── Static colour ────────────────────────────────────────────────────────────
void applyStaticColor() {
  if (!settings.enStatColor) return;
  uint32_t color = strtol(settings.staticColor.substring(1).c_str(), NULL, 16);
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >>  8) & 0xFF;
  uint8_t b =  color        & 0xFF;
  setStatusLED(r, g, b);

  if (settings.ledType == LED_TYPE_WS2812) {
    if (settings.channelMode == CHANNEL_MODE_RGBWW) {
      int total = settings.numOutputs * settings.numLedsPerOutput;
      memset(ledData, 0, sizeof(ledData));
      for (int i = 0; i < total; i++) {
        ledData[i * 5 + 0] = r;
        ledData[i * 5 + 1] = g;
        ledData[i * 5 + 2] = b;
        ledData[i * 5 + 3] = settings.whiteLevel;
        ledData[i * 5 + 4] = settings.white2Level;
      }
      driver.showPixels(NO_WAIT, ledData);

    } else if (settings.channelMode == CHANNEL_MODE_RGBW) {
      for (int i = 0; i < settings.numOutputs; i++) {
        for (int j = 0; j < settings.numLedsPerOutput; j++) {
          strips[i].setPixelColor(j, r, g, b, settings.whiteLevel);
        }
        strips[i].show();
      }

    } else {
      setAllLeds(r, g, b);
    }

  } else {
    setAllLeds(r, g, b);
  }
}

// ─── Colour selection (boot-button standalone mode) ───────────────────────────
void handleColorSelection() {
  if (millis() - state.lastBlinkTime >= 500) {
    state.lastBlinkTime = millis();
    state.displayBlink  = !state.displayBlink;
    if (state.displayBlink)
      displayStatus("Color Selection", "Color: " + presetColors[state.selectedColorIndex]);
    else
      displayStatus("", "");
  }
  uint32_t color = strtol(presetColors[state.selectedColorIndex].substring(1).c_str(), NULL, 16);
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >>  8) & 0xFF;
  uint8_t b =  color        & 0xFF;
  setStatusLED(r, g, b);
  setAllLeds(r, g, b);
}

// ─── Boot button ──────────────────────────────────────────────────────────────
void checkBootButton() {
  if (digitalRead(BOOT_BUTTON_PIN) != LOW) return;
  unsigned long pressTime = millis();

  while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    unsigned long dur = millis() - pressTime;
    if (dur > 1000) {
      // Long press
      if (settings.networkMode == 0) {
        settings.useStaticIP = !settings.useStaticIP;
        saveSettings();
        displayStatus("Switching to", settings.useStaticIP ? "Static IP" : "DHCP",
                      settings.useStaticIP ? settings.staticIP : "");
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        delay(1000); ESP.restart(); return;
      } else if (settings.networkMode == 4) {
        state.colorSelectionMode = !state.colorSelectionMode;
        if (state.colorSelectionMode) {
          for (int i = 0; i < PRESET_COLORS_COUNT; i++) {
            if (settings.staticColor == presetColors[i]) { state.selectedColorIndex = i; break; }
          }
          state.lastBlinkTime = millis();
          state.displayBlink  = true;
          displayStatus("Color Selection", "Color: " + presetColors[state.selectedColorIndex]);
        } else {
          settings.staticColor  = presetColors[state.selectedColorIndex];
          saveSettings();
          state.isStaticColorSet = false;
          state.displayBlink     = false;
          displayStatus("Static Color Mode", "Color: " + settings.staticColor, "Press button to exit");
        }
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        return;
      } else if (settings.networkMode == 3) {
        settings.randomFadeEnabled = !settings.randomFadeEnabled;
        saveSettings();
        displayStatus("RGB Test Mode", settings.randomFadeEnabled ? "Random Fade" : "RGB Cycle", "Long press to toggle");
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) delay(10);
        return;
      } else {
        break;
      }
    }
    delay(10);
  }

  unsigned long dur = millis() - pressTime;
  if (dur > 100) {
    if (settings.networkMode == 4 && state.colorSelectionMode) {
      state.selectedColorIndex = (state.selectedColorIndex + 1) % PRESET_COLORS_COUNT;
      state.isStaticColorSet   = false;
    } else {
      settings.networkMode = (settings.networkMode + 1) % 5;
      switch (settings.networkMode) {
        case 0: settings.useWiFi=false; settings.ledCycleEnabled=false; settings.enStatColor=false; displayStatus("Switching Mode...","Ethernet"); break;
        case 1: settings.useWiFi=true;  settings.ledCycleEnabled=false; settings.enStatColor=false; displayStatus("Switching Mode...","Wi-Fi");    break;
        case 2: settings.useWiFi=false; settings.ledCycleEnabled=false; settings.enStatColor=false; displayStatus("Switching Mode...","AP Mode");   break;
        case 3: settings.ledCycleEnabled=true;  settings.enStatColor=false; displayStatus("Switching Mode...","RGB Test Cycle"); break;
        case 4: settings.ledCycleEnabled=false; settings.enStatColor=true;  displayStatus("Switching Mode...","Static Color");   break;
      }
      saveSettings(); delay(1000); ESP.restart();
    }
  }
}

// ─── WiFi setup ───────────────────────────────────────────────────────────────
void setupWiFi() {
  if (settings.useStaticIP) {
    IPAddress ip, gw, sn;
    if (ip.fromString(settings.staticIP) && gw.fromString(settings.gateway) && sn.fromString(settings.subnet))
      WiFi.config(ip, gw, sn);
  }
  WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
  displayStatus("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    checkBootButton();
    state.ledState = !state.ledState;
    setStatusLED(state.ledState ? 255 : 0, 0, 0);
    displayConnectionStatus("Wi-Fi");
    delay(500);
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/update", HTTP_GET, handleOTAUpdate);
  server.on("/doUpdate", HTTP_POST, handleOTAUpdateResponse, handleDoUpdate);
  server.begin();
  displayStatus("Wi-Fi connected", settings.ssid);
  setStatusLED(0, 255, 0);
}

// ─── Ethernet setup ───────────────────────────────────────────────────────────
void setupEthernet() {
  displayStatus("Initializing Ethernet");
  ESP32_W5500_onEvent();
  ETH.begin(MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST);
  if (settings.useStaticIP) {
    IPAddress ip, gw, sn;
    if (ip.fromString(settings.staticIP) && gw.fromString(settings.gateway) && sn.fromString(settings.subnet))
      ETH.config(ip, gw, sn);
  }
  while (ETH.localIP() == INADDR_NONE) {
    checkBootButton();
    state.ledState = !state.ledState;
    setStatusLED(state.ledState ? 255 : 0, 0, 0);
    displayConnectionStatus("Ethernet");
    delay(500);
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/update", HTTP_GET, handleOTAUpdate);
  server.on("/doUpdate", HTTP_POST, handleOTAUpdateResponse, handleDoUpdate);
  server.begin();
  Serial.println("Ethernet setup complete");
}

// ─── AP mode ──────────────────────────────────────────────────────────────────
void startAPMode() {
  state.isAPMode = true;
  WiFi.softAP(settings.nodeName.c_str());
  IPAddress IP = WiFi.softAPIP();
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/update", HTTP_GET, handleOTAUpdate);
  server.on("/doUpdate", HTTP_POST, handleOTAUpdateResponse, handleDoUpdate);
  server.begin();
  setStatusLED(0, 0, 255);
  displayStatus("AP Mode", settings.nodeName, "IP:" + IP.toString());
}

// ─── CSS ──────────────────────────────────────────────────────────────────────
const char* const CSS_STYLES = R"(
:root {
  --bg-color:#f4f4f4; --text-color:#333; --form-bg:#fff;
  --input-bg:#fff; --input-border:#ccc; --button-bg:#4CAF50;
  --button-hover:#45a049; --accent-color:#007bff;
  --warning-color:#d9534f; --muted-text:#666;
}
body.dark-mode {
  --bg-color:#1a1a1a; --text-color:#e0e0e0; --form-bg:#2d2d2d;
  --input-bg:#3a3a3a; --input-border:#555; --button-bg:#ff8c00;
  --button-hover:#ff7700; --accent-color:#ff8c00;
  --warning-color:#ff6b6b; --muted-text:#999;
}
body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:var(--bg-color);color:var(--text-color);text-align:center;transition:background-color .3s,color .3s;}
h1{font-size:24px;margin-bottom:20px;}
.dark-mode-toggle{position:fixed;top:20px;right:20px;background-color:var(--accent-color);color:#fff;border:none;padding:10px 15px;border-radius:20px;cursor:pointer;font-size:14px;z-index:1000;}
form{background-color:var(--form-bg);padding:20px;padding-right:40px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,.1);max-width:400px;margin:auto;}
input[type='text'],input[type='number']{width:100%;padding:10px;margin:10px 0;border:1px solid var(--input-border);border-radius:4px;font-size:16px;background-color:var(--input-bg);color:var(--text-color);}
input[type='submit']{background-color:var(--button-bg);color:#fff;border:none;padding:12px;font-size:16px;cursor:pointer;border-radius:4px;width:100%;}
input[type='submit']:hover{background-color:var(--button-hover);}
.warning{margin-top:20px;color:var(--warning-color);font-size:14px;}
.pin-input{width:60px!important;display:inline-block;margin-right:10px;}
.pin-container{margin:10px 0;padding:10px;background-color:var(--input-bg);border-radius:5px;}
.pin-label{display:inline-block;width:100px;text-align:left;}
.radio-group{display:flex;justify-content:center;gap:20px;margin:10px 0;flex-wrap:wrap;}
.radio-option{display:flex;align-items:center;gap:5px;}
.slider-container{margin:15px 0;}
.slider-value{display:inline-block;min-width:50px;text-align:center;font-weight:bold;}
input[type='range']{width:70%;}
a{color:var(--accent-color);}
.default-pin{color:var(--muted-text);font-size:.9em;margin-left:5px;}
.section-divider{border-top:1px solid var(--input-border);margin:15px 0;padding-top:10px;font-weight:bold;text-align:left;}
.fade-info{font-size:0.85em;color:var(--muted-text);margin:4px 0 8px 0;text-align:left;}
)";

// ─── JavaScript ───────────────────────────────────────────────────────────────
const char* const JS_CODE = R"(
function togglePinInputs(){
  document.getElementById('pin-container').style.display=
    document.getElementById('enableCustomPins').checked?'block':'none';
}
function toggleStaticIP(){
  document.getElementById('static-ip-container').style.display=
    document.getElementById('useStaticIP').checked?'block':'none';
}
function getChannelMode(){
  return parseInt(document.querySelector('input[name="channelMode"]:checked').value);
}
function updateChannelMode(){
  var cm=getChannelMode();
  var staticColorEnabled=document.getElementById('enStatColor').checked;
  document.getElementById('white-level-container').style.display=
    (cm>=1&&staticColorEnabled)?'block':'none';
  document.getElementById('white2-level-container').style.display=
    (cm===2&&staticColorEnabled)?'block':'none';
  updateUniverseSlider();
}
function updateStaticColorState(){
  var cm=getChannelMode();
  var on=document.getElementById('enStatColor').checked;
  document.getElementById('white-level-container').style.display=(cm>=1&&on)?'block':'none';
  document.getElementById('white2-level-container').style.display=(cm===2&&on)?'block':'none';
}
function updateUniverseSlider(){
  var cm=getChannelMode();
  var ledsPerUniverse=(cm===2)?102:(cm===1?128:170);
  var universes=parseInt(document.getElementById('numLedsSlider').value);
  var totalLeds=universes*ledsPerUniverse;
  document.getElementById('numLedsValue').textContent=
    universes+' universe'+(universes>1?'s':'')+' ('+totalLeds+' LEDs)';
  document.getElementById('numledsoutput').value=totalLeds;
}
function updateOutputSlider(){
  var v=document.getElementById('numOutputSlider').value;
  document.getElementById('numOutputValue').textContent=v;
  document.getElementById('numoutput').value=v;
}
function updateLedTypeUI(){
  var isAPA102=document.querySelector('input[name="ledType"]:checked').value==='1';
  document.getElementById('apa102-clock-info').style.display=isAPA102?'block':'none';
  document.getElementById('rgbw-container').style.display=isAPA102?'none':'block';
  if(isAPA102){
    document.getElementById('ch0').checked=true;
    document.getElementById('white-level-container').style.display='none';
    document.getElementById('white2-level-container').style.display='none';
  }
}
function toggleDarkMode(){
  document.body.classList.toggle('dark-mode');
  var isDark=document.body.classList.contains('dark-mode');
  localStorage.setItem('darkMode',isDark?'enabled':'disabled');
  document.getElementById('darkModeBtn').textContent=isDark?'Light Mode':'Dark Mode';
}
function updateUniverseHint(){
  var sel=document.querySelector('input[name="protocolMode"]:checked');
  var isSACN=sel && sel.value==='1';
  var hidden=document.getElementById('startuniverse');
  var display=document.getElementById('startuniverse_display');
  // Display field shows +1 for sACN, raw value for ArtNet
  display.value=isSACN ? (parseInt(hidden.value)+1) : parseInt(hidden.value);
  document.getElementById('universe-hint').textContent=
    isSACN ? 'sACN universe (1-based). Universe 1 = first universe.' :
              'ArtNet universe (0-based). Universe 0 = first universe.';
}
function syncUniverseInput(){
  var sel=document.querySelector('input[name="protocolMode"]:checked');
  var isSACN=sel && sel.value==='1';
  var hidden=document.getElementById('startuniverse');
  var display=document.getElementById('startuniverse_display');
  // User typed in display field — store raw 0-based value in hidden field
  hidden.value=isSACN ? Math.max(0,parseInt(display.value)-1) : parseInt(display.value);
}

window.onload=function(){
  togglePinInputs();
  updateUniverseHint()
  toggleStaticIP();
  updateChannelMode();
  updateUniverseSlider();
  updateOutputSlider();
  updateLedTypeUI();
  if(localStorage.getItem('darkMode')==='enabled'){
    document.body.classList.add('dark-mode');
    document.getElementById('darkModeBtn').textContent='Light Mode';
  }
};
)";

// ─── Web root page ────────────────────────────────────────────────────────────
void handleRoot() {
  String content = F("<!DOCTYPE html><html><head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<link rel=\"icon\" href=\"data:,\">"
    "<style>");
  content += CSS_STYLES;
  content += F("</style><script>");
  content += JS_CODE;
  content += F("</script></head><body>");
  content += F("<button id='darkModeBtn' class='dark-mode-toggle' onclick='toggleDarkMode()'>Dark Mode</button>");
  content += F("<h1>"); content += settings.nodeName; content += F(" Configuration</h1>");
  content += F("<form method='POST' action='/config'>");

  // ── Node name ────────────────────────────────────────────────────────────
  content += F("<label for='nodename'>Artnet Node name:</label>"
               "<input type='text' id='nodename' name='nodename' value='");
  content += settings.nodeName; content += F("'><br>");

  // ── LED Type selector ─────────────────────────────────────────────────────
  content += F("<div class='section-divider'>LED Strip Type</div>");
  content += F("<div class='radio-group'>");
  content += F("<div class='radio-option'>"
               "<input type='radio' id='ws2812' name='ledType' value='0' onchange='updateLedTypeUI()' ");
  content += (settings.ledType == LED_TYPE_WS2812) ? F("checked") : F("");
  content += F("><label for='ws2812'>WS2812 / NeoPixel (800&nbsp;kHz)</label></div>");
  content += F("<div class='radio-option'>"
               "<input type='radio' id='apa102' name='ledType' value='1' onchange='updateLedTypeUI()' ");
  content += (settings.ledType == LED_TYPE_APA102) ? F("checked") : F("");
  content += F("><label for='apa102'>APA102 / DotStar (SPI)</label></div>");
  content += F("</div>");
  content += F("<div id='apa102-clock-info' style='display:");
  content += (settings.ledType == LED_TYPE_APA102) ? F("block") : F("none");
  content += F(";background:var(--input-bg);border-radius:5px;padding:8px;margin:8px 0;font-size:0.9em;text-align:left;'>"
               "&#9432;&nbsp;APA102 uses a shared SPI clock on GPIO <strong>");
  content += String(APA102_CLOCK_PIN);
  content += F("</strong>. Data pins are configured below."
               " Channel mode selection is not available for APA102 (always RGB).</div>");

  // ── Channel mode ──────────────────────────────────────────────────────────
  content += F("<div id='rgbw-container' style='display:");
  content += (settings.ledType == LED_TYPE_APA102) ? F("none") : F("block");
  content += F(";'>");
  content += F("<div class='section-divider'>Channel Mode</div>");
  content += F("<div class='radio-group'>");

  content += F("<div class='radio-option'>"
               "<input type='radio' id='ch0' name='channelMode' value='0' onchange='updateChannelMode()' ");
  content += (settings.channelMode == CHANNEL_MODE_RGB) ? F("checked") : F("");
  content += F("><label for='ch0'>RGB (3 ch)</label></div>");

  content += F("<div class='radio-option'>"
               "<input type='radio' id='ch1' name='channelMode' value='1' onchange='updateChannelMode()' ");
  content += (settings.channelMode == CHANNEL_MODE_RGBW) ? F("checked") : F("");
  content += F("><label for='ch1'>RGBW (4 ch)</label></div>");

  content += F("<div class='radio-option'>"
               "<input type='radio' id='ch2' name='channelMode' value='2' onchange='updateChannelMode()' ");
  content += (settings.channelMode == CHANNEL_MODE_RGBWW) ? F("checked") : F("");
  content += F("><label for='ch2'>RGBWW / CCT (5 ch)</label></div>");

  content += F("</div></div>");

  content += F("<div class='section-divider'>Protocol</div>");
  content += F("<div class='radio-group'>");
  content += F("<div class='radio-option'>"
               "<input type='radio' name='protocolMode' value='0' onchange='updateUniverseHint()'");
  content += (settings.protocolMode == PROTOCOL_ARTNET) ? F("checked") : F("");
  content += F("><label>ArtNet</label></div>");
  content += F("<div class='radio-option'>"
               "<input type='radio' name='protocolMode' value='1' onchange='updateUniverseHint()'");
  content += (settings.protocolMode == PROTOCOL_SACN) ? F("checked") : F("");
  content += F("><label>sACN (E1.31)</label></div>");
  content += F("<div class='radio-option'>"
               "<input type='radio' name='protocolMode' value='2' onchange='updateUniverseHint()'");
  content += (settings.protocolMode == PROTOCOL_BOTH) ? F("checked") : F("");
  content += F("><label>ArtNet + sACN</label></div>");
  content += F("</div>");

  // ── Universe / LED slider ─────────────────────────────────────────────────
  content += F("<div class='section-divider'>ArtNet Configuration</div>");
  int lpuv             = getLedsPerUniverse();
  int currentUniverses = max(1, min(4, (settings.numLedsPerOutput + lpuv - 1) / lpuv));
  content += F("<div class='slider-container'>"
               "<label for='numLedsSlider'>LEDs per output:</label><br>"
               "<input type='range' id='numLedsSlider' min='1' max='4' step='1' value='");
  content += String(currentUniverses);
  content += F("' oninput='updateUniverseSlider()'>"
               "<span class='slider-value' id='numLedsValue'></span>"
               "<input type='hidden' id='numledsoutput' name='numledsoutput' value='");
  content += String(settings.numLedsPerOutput);
  content += F("'></div>");

  content += F("<div class='slider-container'>"
               "<label for='numOutputSlider'>Number of outputs:</label><br>"
               "<input type='range' id='numOutputSlider' min='1' max='4' step='1' value='");
  content += String(settings.numOutputs);
  content += F("' oninput='updateOutputSlider()'>"
               "<span class='slider-value' id='numOutputValue'>");
  content += String(settings.numOutputs);
  content += F("</span>"
               "<input type='hidden' id='numoutput' name='numoutput' value='");
  content += String(settings.numOutputs);
  content += F("'></div>");

  int displayUniverse = (settings.protocolMode == PROTOCOL_SACN)
                      ? settings.startUniverse + 1
                      : settings.startUniverse;
  content += F("<label for='startuniverse_display'>Start universe:</label>"
              "<input type='text' id='startuniverse_display' value='");
  content += String(displayUniverse);
  content += F("' oninput='syncUniverseInput()'>"
              "<input type='hidden' id='startuniverse' name='startuniverse' value='");
  content += String(settings.startUniverse);
  content += F("'><br>");
  content += F("<div class='fade-info' id='universe-hint'></div>");

  // ── Brightness ───────────────────────────────────────────────────────────
  content += F("<div class='slider-container'>"
               "<label for='ledbrightness'>Max. LED brightness:</label><br>"
               "<input type='range' id='ledbrightness' name='ledbrightness' min='0' max='255' value='");
  content += String(settings.ledBrightness);
  content += F("' oninput='brightnessValue.value=this.value'>"
               "<span class='slider-value'><output id='brightnessValue'>");
  content += String(settings.ledBrightness);
  content += F("</output></span></div>");

  // ── Colour Options ───────────────────────────────────────────────────────
  content += F("<div class='section-divider'>Colour Options</div>");
  content += F("<label for='staticColor'>Static RGB Colour:</label>"
               "<input type='color' id='staticColor' name='staticColor' value='");
  content += settings.staticColor; content += F("'><br>");

  content += F("<label for='enStatColor'>Enable Static Colour:</label>"
               "<input type='checkbox' id='enStatColor' name='enStatColor'"
               " onchange='updateStaticColorState()' ");
  content += settings.enStatColor ? F("checked") : F(""); content += F("><br>");

  // White channel 1 (RGBW and RGBWW)
  bool showW1 = (settings.channelMode >= CHANNEL_MODE_RGBW) && settings.enStatColor
                 && (settings.ledType == LED_TYPE_WS2812);
  content += F("<div id='white-level-container' style='display:");
  content += showW1 ? F("block") : F("none");
  content += F(";'><div class='slider-container'>"
               "<label for='whiteLevel'>White Channel Level (W1):</label><br>"
               "<input type='range' id='whiteLevel' name='whiteLevel' min='0' max='255' value='");
  content += String(settings.whiteLevel);
  content += F("' oninput='whiteLevelValue.value=this.value'>"
               "<span class='slider-value'><output id='whiteLevelValue'>");
  content += String(settings.whiteLevel);
  content += F("</output></span></div></div>");

  // White channel 2 (RGBWW only)
  bool showW2 = (settings.channelMode == CHANNEL_MODE_RGBWW) && settings.enStatColor
                 && (settings.ledType == LED_TYPE_WS2812);
  content += F("<div id='white2-level-container' style='display:");
  content += showW2 ? F("block") : F("none");
  content += F(";'><div class='slider-container'>"
               "<label for='white2Level'>White Channel Level (W2 / CCT):</label><br>"
               "<input type='range' id='white2Level' name='white2Level' min='0' max='255' value='");
  content += String(settings.white2Level);
  content += F("' oninput='white2LevelValue.value=this.value'>"
               "<span class='slider-value'><output id='white2LevelValue'>");
  content += String(settings.white2Level);
  content += F("</output></span></div></div>");

  content += F("<label for='ledCycleEnabled'>Enable RGB Test Cycle:</label>"
               "<input type='checkbox' id='ledCycleEnabled' name='ledCycleEnabled' ");
  content += settings.ledCycleEnabled ? F("checked") : F(""); content += F("><br>");

  // ── Colour Fade (sequential) ──────────────────────────────────────────────
  content += F("<label for='randomFadeEnabled'>Enable Colour Fade Cycle:</label>"
               "<input type='checkbox' id='randomFadeEnabled' name='randomFadeEnabled' ");
  content += settings.randomFadeEnabled ? F("checked") : F(""); content += F("><br>");

  // ── Custom pins ───────────────────────────────────────────────────────────
  content += F("<div class='section-divider'>Pin Configuration</div>");
  content += F("<label for='enableCustomPins'>Enable Custom Pins:</label>"
               "<input type='checkbox' id='enableCustomPins' name='enableCustomPins'"
               " onchange='togglePinInputs()' ");
  content += settings.enableCustomPins ? F("checked") : F(""); content += F("><br>");

  content += F("<div id='pin-container' class='pin-container'>");
  for (int i = 0; i < NUMSTRIPS; i++) {
    content += F("<div><span class='pin-label'>Output ");
    content += String(i + 1);
    content += F(" Data Pin:</span>"
                 "<input type='number' class='pin-input' name='pin");
    content += String(i);
    content += F("' min='0' max='39' value='");
    content += String(settings.pins[i]);
    content += F("'><span class='default-pin'>(Default: ");
    content += String(DEFAULT_PINS[i]);
    content += F(")</span></div>");
  }
  content += F("</div>");

  // ── Network ───────────────────────────────────────────────────────────────
  content += F("<div class='section-divider'>Network</div>");
  content += F("<label for='useStaticIP'>Use Static IP:</label>"
               "<input type='checkbox' id='useStaticIP' name='useStaticIP'"
               " onchange='toggleStaticIP()' ");
  content += settings.useStaticIP ? F("checked") : F(""); content += F("><br>");

  content += F("<div id='static-ip-container' style='display:none;'>");
  content += F("<label>Static IP:</label><input type='text' name='staticIP' value='");
  content += settings.staticIP; content += F("' placeholder='192.168.1.100'><br>");
  content += F("<label>Gateway:</label><input type='text' name='gateway' value='");
  content += settings.gateway; content += F("' placeholder='192.168.1.1'><br>");
  content += F("<label>Subnet:</label><input type='text' name='subnet' value='");
  content += settings.subnet; content += F("' placeholder='255.255.255.0'><br></div>");

  content += F("<label for='useWiFi'>Use Wi-Fi:</label>"
               "<input type='checkbox' id='useWiFi' name='useWiFi' ");
  content += settings.useWiFi ? F("checked") : F(""); content += F("><br>");
  content += F("<label>SSID:</label><input type='text' name='ssid' value='");
  content += settings.ssid; content += F("'><br>");
  content += F("<label>Password:</label><input type='text' name='password' value='");
  content += settings.password; content += F("'><br>");

  content += F("<input type='submit' value='Save &amp; Restart'>");
  content += F("</form>");
  content += F("<div class='warning'><p><strong>Warning:</strong> Ensure node settings "
               "match your software (pixel count / universes).</p></div>");
  content += F("<div style='margin-top:20px;'>"
               "<a href='/update' style='background-color:#007bff;color:#fff;padding:10px 15px;"
               "text-decoration:none;border-radius:4px;display:inline-block;'>Firmware Update</a>"
               "</div>");
  content += F("</body></html>");
  server.send(200, "text/html", content);
}

// ─── Config POST handler ──────────────────────────────────────────────────────
void handleConfig() {
  if (server.method() != HTTP_POST) return;
  if (server.hasArg("protocolMode"))  settings.protocolMode     = server.arg("protocolMode").toInt();
  if (server.hasArg("numledsoutput")) settings.numLedsPerOutput = server.arg("numledsoutput").toInt();
  if (server.hasArg("numoutput"))     settings.numOutputs       = server.arg("numoutput").toInt();
  if (server.hasArg("startuniverse"))  settings.startUniverse   = server.arg("startuniverse").toInt();
  if (server.hasArg("nodename"))      settings.nodeName         = server.arg("nodename");
  if (server.hasArg("ledbrightness")) settings.ledBrightness    = server.arg("ledbrightness").toInt();
  if (server.hasArg("ssid"))          settings.ssid             = server.arg("ssid");
  if (server.hasArg("password"))      settings.password         = server.arg("password");
  if (server.hasArg("staticColor"))   settings.staticColor      = server.arg("staticColor");
  if (server.hasArg("whiteLevel"))    settings.whiteLevel       = server.arg("whiteLevel").toInt();
  if (server.hasArg("white2Level"))   settings.white2Level      = server.arg("white2Level").toInt();

  // LED type
  if (server.hasArg("ledType"))
    settings.ledType = server.arg("ledType").toInt();

  // Channel mode (only meaningful for WS2812)
  if (settings.ledType == LED_TYPE_APA102) {
    settings.channelMode = CHANNEL_MODE_RGB;
  } else if (server.hasArg("channelMode")) {
    int newMode = server.arg("channelMode").toInt();
    if (newMode < CHANNEL_MODE_RGB || newMode > CHANNEL_MODE_RGBWW) newMode = CHANNEL_MODE_RGB;
    if (settings.channelMode != newMode) {
      settings.channelMode = newMode;
      int maxLeds = getMaxLedsPerStrip();
      if (settings.numLedsPerOutput > maxLeds) settings.numLedsPerOutput = maxLeds;
    }
  }

  settings.useWiFi          = server.hasArg("useWiFi");
  settings.enStatColor       = server.hasArg("enStatColor");
  settings.ledCycleEnabled   = server.hasArg("ledCycleEnabled");
  settings.randomFadeEnabled = server.hasArg("randomFadeEnabled");   // NEW
  settings.networkMode       = settings.useWiFi ? 1 : 0;

  bool wasCustom = settings.enableCustomPins;
  settings.enableCustomPins = server.hasArg("enableCustomPins");
  settings.useStaticIP      = server.hasArg("useStaticIP");
  if (server.hasArg("staticIP")) settings.staticIP = server.arg("staticIP");
  if (server.hasArg("gateway"))  settings.gateway  = server.arg("gateway");
  if (server.hasArg("subnet"))   settings.subnet   = server.arg("subnet");

  bool pinsChanged = false;
  if (wasCustom && !settings.enableCustomPins) { resetToDefaultPins(); pinsChanged = true; }
  if (settings.enableCustomPins) {
    for (int i = 0; i < NUMSTRIPS; i++) {
      String key = "pin" + String(i);
      if (server.hasArg(key)) {
        int np = server.arg(key).toInt();
        if (settings.pins[i] != np) { settings.pins[i] = np; pinsChanged = true; }
      }
    }
  }

  // Priority resolution: randomFade wins, then staticColor, then cycle
  if (settings.randomFadeEnabled) {
    settings.enStatColor     = false;
    settings.ledCycleEnabled = false;
  } else if (settings.ledCycleEnabled && settings.enStatColor) {
    settings.ledCycleEnabled = false; // static colour wins over cycle
  }

  // Reset fade state so it re-initialises with fresh RNG on next boot
  fadeState.initialised = false;

  state.isStaticColorSet = false;
  saveSettings();

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
  delay(500);
  ESP.restart();
}

// ─── OTA pages ────────────────────────────────────────────────────────────────
void handleOTAUpdate() {
  String content = F("<!DOCTYPE html><html><head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<link rel=\"icon\" href=\"data:,\"><style>");
  content += CSS_STYLES;
  content += F("</style></head><body>");
  content += F("<h1>"); content += settings.nodeName; content += F(" - OTA Update</h1>");
  content += F("<form method='POST' action='/doUpdate' enctype='multipart/form-data'>"
               "<p>Select firmware .bin file:</p>"
               "<input type='file' name='update' accept='.bin'><br><br>"
               "<input type='submit' value='Upload and Update'></form>");
  content += F("<div class='warning'><p><strong>Warning:</strong> Uploading incorrect firmware may brick your device.</p></div>");
  content += F("<p><a href='/'>Return to configuration</a></p></body></html>");
  server.send(200, "text/html", content);
}

void handleDoUpdate() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    displayStatus("OTA Update", "Starting...");
    setStatusLED(120, 0, 120);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
      setStatusLED(255, 0, 0);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
      setStatusLED(255, 0, 0);
    } else {
      float p = (float)Update.progress() / (float)Update.size() * 100.0f;
      displayStatus("OTA Update", "Progress: " + String(p, 1) + "%");
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      displayStatus("OTA Update", "Complete!", "Rebooting...");
      setStatusLED(0, 255, 0);
    } else {
      Update.printError(Serial);
      setStatusLED(255, 0, 0);
    }
  }
  yield();
}

void handleOTAUpdateResponse() {
  if (Update.hasError()) {
    server.send(200, "text/html",
      "<h1>Update Failed!</h1><p><a href='/update'>Try again</a></p>");
  } else {
    server.send(200, "text/html",
      "<!DOCTYPE html><html><head>"
      "<meta http-equiv='refresh' content='5;url=/' /></head>"
      "<body><h1>Update Successful!</h1><p>Rebooting in 5 seconds...</p></body></html>");
    delay(1000);
    ESP.restart();
  }
}
