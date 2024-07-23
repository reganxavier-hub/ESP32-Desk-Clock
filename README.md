# ESP32-Desk-Clock

This project is an NTP-based clock using an ESP32 and a 4-digit 7-segment display controlled via 74HC595 shift registers.

## Author
Infant Regan

## Hardware Required
- ESP32
- 4-digit 7-segment display
- 74HC595 shift register
- Wi-Fi network

## Getting Started

### Prerequisites
- Arduino IDE with ESP32 support installed.
- Led4digit74HC595 library installed.

### Installation
1. Clone this repository:
   ```sh
   git clone https://github.com/your-username/ESP32-NTP-Clock.git
   cd ESP32-NTP-Clock
2. Open ESP32_NTP_Clock.ino in Arduino IDE.
2. Update the following lines with your network details:
     const char* ssid = "your-SSID";
     const char* password = "your-PASSWORD";
3. Upload the code to your ESP32.

### Usage
Connect your ESP32 to your network.
Open a Serial Monitor to view the output.
The current time will be displayed on the 4-digit 7-segment display.
