# ESP32-Artnet-Node-receiver

![Image Description](https://github.com/mdethmers/ESP32-Artnet-Node-receiver/blob/main/Img/IMG_20250623_115355.jpg)

This Artnet ESP32 LED controller is a high-performance, open-source solution for controlling addressable LEDs using Ethernet or Wi-Fi. This project allows you to control multiple LED strips using the Artnet protocol via an ESP32 microcontroller with a W5500 Ethernet module or ESP32-ETH01. The code includes a simple web server for configuration and control of the LED strips. For optimal performance, use the W5500 as the ETH01 suffers from performance issues. 

NOW FOR SALE: https://www.etsy.com/nl/shop/LichtBit?ref=l2-about-shopname&from_page=listing

Soldering Tutorial: https://www.youtube.com/watch?v=-qxbHJJCwbM
Node & Resolume Arena configuration: https://www.youtube.com/watch?v=1a7sWZTA7lg

## Features

- 4 Outputs, 16 Universes in Total: Supports up to 4 LED strips with 4 universes per output, enabling large-scale LED installations.
- Ethernet and Wi-Fi Support: Supports both wired and wireless Art-Net control for flexible connectivity.
- AP Mode for Debugging: Easily configure the device in standalone mode without needing an external network.
- Static IP option for permanent installations.
- Physical button for Mode switching
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
 
## Supported LED ICs
All LEDs based on the wsXXXX series are supported. These can be 3, 4 or even 5 channel LEDs (ARTNET ONLY FOR NOW), as long as they have  single data line and 800Khz timing. These include but are not limited to:
  - WS2805, WS2811, WS2812, WS2813, WS2814, WS2815 (Tested)
  - SK6812, SK6812W, SK6812-mini (Not tested)
  - GS8208, TM1814, TM1829, UCS8904, UCS8903, FW1906 (Not tested)

Tests with SPI based leds like the APA102/107 are a work in progress, just like software support for 4 and 5 channel leds. Artnet works with 4/5 channel leds, but software support is coming!
  
 
# Configuration

![Image Description](https://github.com/mdethmers/ESP32-W5500-Artnet-receiver/blob/main/Img/Schermafbeelding%202025-05-13%20224004.png)

## Access the Web Interface:
- After uploading the code, connect your ESP32 to a network using the W5500 Ethernet module.
- Set core debug level to "info" for extra infomration on performance
- Find the IP address assigned to the ESP32 and open it in a web browser.
  - To find the IP address easily, use Resolume advanced output. It will autodetect the Artnet node.
 
## Mode switching with the Boot button
The code allows you to use the boot button to switch between Ethernet, Wifi, AP, RGB Test cycle, and a static colour. In some of the modes, a long press allows you to change more settings by single pressing. Exit by long-pressing again.  
- Ethernet > Long press > Switch between static IP / DCHP (Not implemented yet)
- Wifi > Long press > Switch between static IP / DCHP (Not implemented yet)
- RGB Test cycle > Long press > Switch to other test pattern (Not implemented yet)
- Static colour > Long press > Switch between 10 different colours (WORKING!)

Change this code with your own colours before compiling. 
```
const String presetColors[PRESET_COLORS_COUNT] = {
  "#FF0000", // Red
  "#00FF00", // Green  
  "#0000FF", // Blue
  "#FFFF00", // Yellow
  "#FF00FF", // Magenta
  "#00FFFF", // Cyan
  "#FFFFFF", // White
  "#FF8000", // Orange
  "#8000FF", // Purple
  "#000000"  // Black/Off
};
```
 
## OTA updates
![Image Description](https://github.com/mdethmers/ESP32-W5500-Artnet-receiver/blob/main/Img/Schermafbeelding%202025-05-13%20224014.png)

Upload the BIN file to your Node to have OTA updates. Easy when your node is in a hard to reach or remote place!

 
## Access the AP mode:
- After 30 seconds without any Ethernet response, an AP mode will automatically launch
- You can use this to connect to the Node and set up settings or a Wifi connection. 

## Configure the LED Controller:
Important! Be sure that the Universes/outputs/leds always match what you output. A mismatch can result in a black output. 

## Pin/Setting/Config info
- Default led strip pins are 12, 14, 27, 26 (more outputs can be added if necessary. Change config accordingly).
- Default status led is set to pin 16.
- Pin 16/17 can be reprogrammed for new functionality/adding more outputs. (They are connected to the 5v level shifter). 
- I2C pins for OLED are set to pins 21 and 22.
- The default timeout is set to 30 seconds.
- There is a 5V switch Where the 12v regulator sits. Use this if you power the node with 5v Intead of 12V! The esp and other logic will be powered from the 5v power supply instead of the 12v to 5v regulator. 

### A note on direct Ethernet connection between Node and PC
Sadly, the W5500 does not support Auto-MDI/MDIX, which means you need a crossover cable whenever you want to connect directly from your PC to the Node with an Ethernet cable!

#Basic resolume setup example
To get you up and running with your node, I provide a basic setup example for Resolume. Other software will work similarly.
1. Connect your node to a router (DHCP) or switch (Static IP) through Ethernet, wifi, or AP mode (192.168.4.1).
2. Once your node is successfully connected to your network, navigate to the configuration page (192.168.1.xxx) and set the Node's Name, Number of outputs, and MAXIMUM Leds you connect to your output.
3. Optional) Enable the RGB test cycle or Static color to test if all ouputs and LEDs behave as expected.
4. In Resolume, set the correct network adapter in Preferences>DMX.
5. In Resolume's advanced output, add a new Lumiverse and add the LED strips.
6. In Resolume's advanced output, set the IP address.
7. The node supports 4 universes (680 LEDS) per output. If you need all 16 universes (680 leds per output), add 4 Lumiverse outputs and set the right universes per output. For output 1 that is start universe 0, For output 2 that is start universe 4, For output 3 that is start universe 8, For output 4 that is start universe 12. Change this corresponding to the number of universes per output that you need.
8. Your strips should now light up!

## Open-Source Contributions
This project is fully open-source! Feel free to contribute improvements, share your builds, and collaborate with the community.

##License
This project is released under the MIT License. You are free to use, modify, and distribute it with proper attribution.

