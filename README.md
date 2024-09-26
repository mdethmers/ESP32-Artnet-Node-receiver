# ESP32-W5500-Artnet-receiver

![Image Description](https://github.com/mdethmers/ESP32-W5500-Artnet-receiver/blob/main/Img/Schermafbeelding%202024-09-26%20162838.png)


This project allows you to control multiple LED strips using the Artnet protocol via an ESP32 microcontroller with a W5500 Ethernet module. The code includes a simple web server for configuration and control of the LED strips.

## Features

- **Artnet Protocol**: Communicate with your LED strips using the Artnet protocol.
- **Web Interface**: Configure the number of LEDs, outputs, start universe, and node name via a web interface.
- **Non-Volatile Storage**: Save and load configuration settings using the Preferences library.
- **Ethernet Connectivity**: Use the W5500 Ethernet module for network communication.

## PCB Design
This is the ongoing PCB design, using 12v-to-5v stepdown converters, Polyfuses for protection and screwterminals for easy connections. 
Project: https://oshwlab.com/matthijsdethmers/artnetnode-esp32
EasyEDA Editor: https://easyeda.com/editor#project_id=86fd5b1121594bfa85fd2c5eed017e20

## Hardware Requirements

- ESP32-Wroom-Dev1 microcontroller 
- W5500 Ethernet module
- LED strips compatible with I2SClockless LED driver

**Important!** Only a few ESP32's are supported! Check the WebServer_ESP32_W5500 for supported Boards
  
## Software Requirements

- Arduino IDE with ESP32 support
- Libraries:
  - **Preferences** https://www.arduino.cc/reference/en/libraries/preferences/
  - **WebServer_ESP32_W5500** https://github.com/khoih-prog/WebServer_ESP32_W5500
  - **I2SClocklessLedDriver** https://github.com/hpwit/I2SClocklessLedDriver
  - **artnetESP32V2** https://github.com/hpwit/artnetesp32v2
 
# Configuration
## Access the Web Interface:
- After uploading the code, connect your ESP32 to a network using the W5500 Ethernet module.
- Find the IP address assigned to the ESP32 and open it in a web browser.

## Configure the LED Controller:
- Use the web interface to set the following parameters:
-- Number of LEDs per output: The maximum number of LEDs per strip.
-- Number of outputs: The number of LED strips.
-- Start universe: The Artnet universe ID to start with.
-- Artnet Node name: The name of the Artnet node.
-- Click Save to apply the changes. The settings will be stored and used by the ESP32.

## Code Overview
- displayfunction(void *param): Function to update the LED strips with data received from Artnet.
- handleRoot(): Serves the main configuration page via HTTP.
- handleConfig(): Processes configuration updates from the web interface.
- setup(): Initializes serial communication, preferences, LED driver, Ethernet, Artnet, and web server.
- loop(): Handles incoming HTTP requests.

