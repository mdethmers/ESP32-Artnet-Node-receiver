# ESP32-Artnet-Node-receiver

![Image Description](https://github.com/mdethmers/ESP32-W5500-Artnet-receiver/blob/main/Img/Schermafbeelding%202025-05-13%20224004.png)



NOW FOR SALE: https://www.etsy.com/nl/shop/LichtBit?ref=l2-about-shopname&from_page=listing

This Artnet ESP32 LED controller is a high-performance, open-source solution for controlling addressable LEDs using Ethernet or Wi-Fi. This project allows you to control multiple LED strips using the Artnet protocol via an ESP32 microcontroller with a W5500 Ethernet module or ESP32-ETH01. The code includes a simple web server for configuration and control of the LED strips. For optimal performance, use the W5500 as the ETH01 suffers from performance issues. 

## Features

- 4 Outputs, 16 Universes in Total: Supports up to 4 LED strips with 4 universes per output, enabling large-scale LED installations.
- Ethernet and Wi-Fi Support: Supports both wired and wireless Art-Net control for flexible connectivity.
- AP Mode for Debugging: Easily configure the device in standalone mode without needing an external network.
- Static IP option for permanent installations.
- Over-The-Air (OTA) updates to update your node with different versions easily.
- Configure the number of LEDs, outputs, start universe, and node name via a web interface.
- High Frame Rates: Over 40 FPS, depending on the software used.
- Reliable Performance: Boasts 99%+ reliability with less than 1% dropped frames.
- 128x32 OLED Display Support: Provides real-time status updates and configuration feedback.
- Status RGB LED: Indicates device status for easy debugging and monitoring.
- **RGB Test Cycle**: Quickly test LED outputs with built-in cycling modes.
- Fully Open-Source: Includes PCB design and BOM, allowing users to build their own cost-effective controllers.

## PCB Design
This is the ongoing PCB design, using 12v-to-5v step-down converters, Car-fuses for protection and screw terminals for easy connections. 
Project: https://oshwlab.com/matthijsdethmers/artnetnode-esp32
EasyEDA Editor: https://easyeda.com/editor#project_id=86fd5b1121594bfa85fd2c5eed017e20

## Hardware Requirements

- ESP32-Wroom-Dev1 (or ESP32-ETH01 microcontroller)
- W5500 Ethernet module
- LED strips compatible with I2SClockless LED driver (Tested on WS2811, WS2812(B), WS2815)
- 128x32 OLED Display (Optional for visual feedback)
- WS2812 RGB Status LED (Optional for status indication)
- 12v/5v Power Supply

**Important!** Only a few ESP32's are supported! Check the WebServer_ESP32_W5500 for supported Boards when using the W5500.
  
## Software Requirements

- Arduino IDE with ESP32 support
- Espressif ESP32 board manager library
  - Use https://espressif.github.io/arduino-esp32/package_esp32_index.json
  - Version 2.0.17 or lower, otherwise compilation errors arise
- Libraries:
  - **Preferences** https://www.arduino.cc/reference/en/libraries/preferences/
  - **WebServer_ESP32_W5500** https://github.com/khoih-prog/WebServer_ESP32_W5500
  - **I2SClocklessLedDriver** https://github.com/hpwit/I2SClocklessLedDriver
  - **artnetESP32V2** https://github.com/hpwit/artnetesp32v2
  - WebServer.h (For Wi-Fi and web interface support)
  - Adafruit_NeoPixel.h (For addressable LED control)
  - SPI.h, Wire.h (Built-in libraries for communication)
  - Adafruit_GFX.h (Graphics library for OLED)
  - Adafruit_SSD1306.h (OLED display driver)
  - Update.h (OTA updates)
 
# Configuration
## Access the Web Interface:
- After uploading the code, connect your ESP32 to a network using the W5500 Ethernet module.
- Set core debug level to "info" for extra infomration on performance
- Find the IP address assigned to the ESP32 and open it in a web browser.
  - To find the IP address easily, use Resolume advanced output. It will autodetect the Artnet node.
 
## OTA updates
![Image Description](https://github.com/mdethmers/ESP32-W5500-Artnet-receiver/blob/main/Img/Schermafbeelding%202025-05-13%20224014.png)

Upload the BIn file to your Node to have OTA updates. Easy when your node is in a hard to reach or remote place!

 
## Access the AP mode:
- After 30 seconds without any Ethernet response, an AP mode will automatically launch
- You can use this to connect to the Node and set up settings or a Wifi connection. 

## Configure the LED Controller:
Important! Be sure that the Universes/outputs/leds always match with what you output. A mismatch can result in a black output. 

## Pin/Setting/Config info
- Default led strip pins are 12, 14, 27, 26 (more outputs can be aded if necessary. change config accordingly)
- Default status led is set to pin 16
- i2c pins for oled is set to pin 21 and 22
- default timeout is set to 30 sec

## Open-Source Contributions
This project is fully open-source! Feel free to contribute improvements, share your builds, and collaborate with the community.

##License
This project is released under the MIT License. You are free to use, modify, and distribute it with proper attribution.

