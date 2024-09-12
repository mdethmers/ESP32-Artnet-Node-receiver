// Define debugging and configuration settings
#define DEBUG_ETHERNET_WEBSERVER_PORT Serial
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 3
#define UNIQUE_SUBARTNET

// Define configuration constants
#define NUM_LEDS_PER_STRIP 680 // Maximum number of LEDs per strip
#define NUMSTRIPS 3 // Number of LED strips
#define NB_CHANNEL_PER_LED 3  // Number of channels per LED (3 for RGB, 4 for RGBW)
#define UNIVERSE_SIZE_IN_CHANNEL 510  // Size of a universe (170 pixels * 3 channels)
#define COLOR_GRB // Define color order (Green-Red-Blue)

// Include necessary libraries
#include <Preferences.h>
#include "WebServer_ESP32_W5500.h" // Web server library for ESP32 with W5500 Ethernet
#include "I2SClocklessLedDriver.h" // Library to drive LEDs via I2S
#include "Arduino.h"
#include "artnetESP32V2.h" // Artnet library for ESP32

// Create a web server object listening on port 80
WebServer server(80);
Preferences preferences; // Object to handle non-volatile storage

// Default configuration values
int numledsperoutput = 680; // Default number of LEDs per output
int numoutput = 3; // Default number of outputs (LED strips)
int startuniverse = 0; // Default starting universe ID
int pins[NUMSTRIPS] = { 2, 15, 0}; // GPIO pins connected to each LED strip
String nodename = "Arnet Node esp32"; // Default node name

// Create instances of Artnet and LED driver
artnetESP32V2 artnet = artnetESP32V2();
I2SClocklessLedDriver driver;

// Function to display LED data
void displayfunction(void *param){
    // Cast the parameter to the subArtnet structure
    subArtnet *subartnet = (subArtnet *)param;
    // Update LEDs with the received data
    driver.showPixels(NO_WAIT, subartnet->data);
}

// Function to handle the root page of the web server
void handleRoot() {
  String content = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  content += "<link rel=\"icon\" href=\"data:,\">"; // Empty favicon
  content += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  content += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
  content += ".button2 { background-color: #555555; }</style></head>";
  content += "<body><h1>"+ nodename +" Configuration</h1><form method='get' action='/config'>";
  content += "Number of LEDs per output: <input type='text' name='numledsperoutput' value='" + String(numledsperoutput) + "'><br>";
  content += "Number of outputs: <input type='text' name='numoutput' value='" + String(numoutput) + "'><br>";
  content += "Start universe: <input type='text' name='startuniverse' value='" + String(startuniverse) + "'><br>";
  content += "Artnet Node name: <input type='text' name='nodename' value='" + nodename + "'><br>";
  content += "<input type='submit' value='Save'></form></body></html>";
  server.send(200, "text/html", content); // Send HTML response
}

// Function to handle the configuration updates
void handleConfig() {
  if (server.args() > 0) {
    // Process each argument from the HTTP GET request
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "numledsperoutput") {
        numledsperoutput = server.arg(i).toInt(); // Update number of LEDs per output
      } else if (server.argName(i) == "numoutput") {
        numoutput = server.arg(i).toInt(); // Update number of outputs
      } else if (server.argName(i) == "startuniverse") {
        startuniverse = server.arg(i).toInt(); // Update starting universe ID
      } else if (server.argName(i) == "nodename") {
        nodename = server.arg(i); // Update node name
      }
    }
    
    // Save updated configuration to non-volatile storage
    preferences.begin("esp32config", false);
    preferences.putInt("numledsperoutput", numledsperoutput);
    preferences.putInt("numoutput", numoutput);
    preferences.putInt("startuniverse", startuniverse);
    preferences.putString("nodename", nodename);
    preferences.end();
  }
  
  handleRoot(); // Show the updated root page
}

void setup() {
  Serial.begin(115200); // Start serial communication

  // Initialize Preferences to load saved configuration
  preferences.begin("esp32config", true);
  numledsperoutput = preferences.getInt("numledsperoutput", numledsperoutput);
  numoutput = preferences.getInt("numoutput", numoutput);
  startuniverse = preferences.getInt("startuniverse", startuniverse);
  nodename = preferences.getString("nodename", nodename);
  preferences.end();

  // Wait for Serial to be available (for debugging)
  while (!Serial && (millis() < 5000));

  // Initialize LED driver
  driver.initled(NULL, pins, NUMSTRIPS, NUM_LEDS_PER_STRIP);
  driver.setBrightness(20); // Set LED brightness

  // Initialize Ethernet
  ESP32_W5500_onEvent(); // Handle Ethernet events
  ETH.begin(MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST); // Start Ethernet interface

  // Setup Artnet with the specified configuration
  artnet.addSubArtnet(startuniverse, numledsperoutput * numoutput * NB_CHANNEL_PER_LED, UNIVERSE_SIZE_IN_CHANNEL, &displayfunction);
  artnet.setNodeName(nodename); // Set Artnet node name

  ESP32_W5500_waitForConnect(); // Wait for DHCP IP assignment

  if (artnet.listen(ETH.localIP(), 6454)) {
    Serial.print("artnet Listening on IP: ");
    Serial.println(ETH.localIP()); // Print assigned IP address
  }

  // Initialize Web Server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_GET, handleConfig);
  server.begin(); // Start the web server

  Serial.println("HTTP server started"); // Indicate server start
}

void loop() {
  server.handleClient(); // Handle incoming HTTP requests
}
