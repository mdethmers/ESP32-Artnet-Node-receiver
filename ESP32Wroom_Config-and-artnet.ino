// Define debugging and configuration settings
#define DEBUG_ETHERNET_WEBSERVER_PORT Serial
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 3
#define UNIQUE_SUBARTNET

// Define configuration constants
#define NUM_LEDS_PER_STRIP 680 // Maximum number of LEDs per strip
#define NUMSTRIPS 4 // Number of LED strips
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
int numledsoutput = 680; // Default number of LEDs per output
int numoutput = 3; // Default number of outputs (LED strips)
int startuniverse = 0; // Default starting universe ID
int pins[NUMSTRIPS] = { 2, 15, 0, 17}; // GPIO pins connected to each LED strip
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
  content += "<style>";
  content += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; color: #333; text-align: center; }";
  content += "h1 { font-size: 24px; margin-bottom: 20px; }";
  content += "form { background-color: #fff; padding: 20px; padding-right: 40px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }";
  content += "input[type='text'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 4px; font-size: 16px; }";
  content += "input[type='submit'] { background-color: #4CAF50; color: white; border: none; padding: 12px; font-size: 16px; cursor: pointer; border-radius: 4px; width: 100%; }";
  content += "input[type='submit']:hover { background-color: #45a049; }";
  content += ".warning { margin-top: 20px; color: #d9534f; font-size: 14px; }"; // Warning text style
  content += "</style></head>";
  
  content += "<body><h1>" + nodename + " Configuration</h1>";
  content += "<form method='POST' action='/config'>";
  content += "<label for='numledsoutput'>Number of LEDs per output (Max. 680):</label>";
  content += "<input type='text' id='numledsoutput' name='numledsoutput' value='" + String(numledsoutput) + "'><br>";
  
  content += "<label for='numoutput'>Number of outputs (Max. 4):</label>";
  content += "<input type='text' id='numoutput' name='numoutput' value='" + String(numoutput) + "'><br>";
  
  content += "<label for='startuniverse'>Start universe:</label>";
  content += "<input type='text' id='startuniverse' name='startuniverse' value='" + String(startuniverse) + "'><br>";
  
  content += "<label for='nodename'>Artnet Node name:</label>";
  content += "<input type='text' id='nodename' name='nodename' value='" + nodename + "'><br>";
  
  content += "<input type='submit' value='Save'>";
  content += "</form>";

  // Warning section
  content += "<div class='warning'>";
  content += "<p><strong>Warning:</strong> Please ensure that the settings on the node (number of pixels/universes) are the same as in your software to prevent errors.</p>";
  content += "</div>";
  
  content += "</body></html>";
  
  server.send(200, "text/html", content); // Send HTML response
}

// Function to handle the configuration updates
void handleConfig() {
  if (server.method() == HTTP_POST) {
    // Retrieve the values from POST request
    if (server.hasArg("numledsoutput")) {
      numledsoutput = server.arg("numledsoutput").toInt();
    }
    if (server.hasArg("numoutput")) {
      numoutput = server.arg("numoutput").toInt();
    }
    if (server.hasArg("startuniverse")) {
      startuniverse = server.arg("startuniverse").toInt();
    }
    if (server.hasArg("nodename")) {
      nodename = server.arg("nodename");
    }
    
    // Save updated configuration to non-volatile storage
    preferences.begin("esp32config", false);
    preferences.putInt("numledsoutput", numledsoutput);
    preferences.putInt("numoutput", numoutput);
    preferences.putInt("startuniverse", startuniverse);
    preferences.putString("nodename", nodename);
    preferences.end();

    // Send a redirection response to the root page (or a clean page)
    server.sendHeader("Location", "/", true); // Redirect to root
    server.send(302, "text/plain", ""); // HTTP 302 redirect
    } else {
      server.send(405, "text/plain", "Method Not Allowed");
    }
  
  ESP.restart();
}

void setup() {
  Serial.begin(115200); // Start serial communication

  // Initialize Preferences to load saved configuration
  preferences.begin("esp32config", true);
  numledsoutput = preferences.getInt("numledsoutput", numledsoutput);
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
  artnet.addSubArtnet(startuniverse, numledsoutput * numoutput * NB_CHANNEL_PER_LED, UNIVERSE_SIZE_IN_CHANNEL, &displayfunction);
  artnet.setNodeName(nodename); // Set Artnet node name

  ESP32_W5500_waitForConnect(); // Wait for DHCP IP assignment

  if (artnet.listen(ETH.localIP(), 6454)) {
    Serial.print("artnet Listening on IP: ");
    Serial.println(ETH.localIP()); // Print assigned IP address
  }

  // Initialize Web Server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.begin(); // Start the web server

  Serial.println("HTTP server started"); // Indicate server start
}

void loop() {
  server.handleClient(); // Handle incoming HTTP requests
}
