# Robot Trash Sorter
The Robot Trash Sorter is an innovative IoT project that combines robotics, sensor technology, and web-based control to automate the process of sorting metal and non-metal trash. This system demonstrates the practical application of various technologies in waste management and automation. The main control system uses an ESP32 microcontroller, while an Arduino Uno controls the conveyor belt.

## Features

- Sorts metal and non-metal trash
- Web-based control interface
- Telegram notifications for bin fullness
- Ultrasonic sensors for waste level detection
- Proximity sensor for metal detection
- Servo motor for trash sorting
- DC motors for robot movement
- Adjustable speed control

## Hardware Requirements
### Main Control System (ESP32)
- ESP32 development board
- 2x Ultrasonic sensors (HC-SR04 or similar)
- 1x Proximity sensor
- 1x Servo motor
- 4x DC motors (for robot movement)
- L298N motor driver module
- Power supply (appropriate for your motors)

### Conveyor Belt System (Arduino Uno)

- Arduino Uno
- L298N motor driver module
- DC motor(s) for the conveyor belt
- Potentiometer for speed control

## Software Dependencies/Libraries
### ESP32
- NewPing library
- WiFi library
- AsyncTCP library
- ESPAsyncWebServer library
- UniversalTelegramBot library
- WiFiClientSecure library
- ESP32Servo library

### Arduino Uno
- No additional libraries are required for the conveyor belt control.

## Setup and Configuration

- Install the required libraries for the ESP32 project.
- Update the WiFi credentials in the ESP32 code:
```
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
```
- Set up a Telegram bot and update the bot token and chat ID:
```
#define BOT_TOKEN "your_bot_token"
#define CHAT_ID "your_chat_id"
```
- Adjust pin assignments if necessary for both ESP32 and Arduino Uno codes.
- Upload the ESP32 code to your ESP32 board.
- Upload the Arduino code to your Arduino Uno.

## Usage

- Power on the robot and ensure it's connected to WiFi.
- Access the web interface by navigating to the ESP32's IP address in a web browser.
- Use the web interface to control the robot's movement and monitor waste bin levels.
- The conveyor belt speed can be adjusted using the potentiometer connected to the Arduino Uno.
- The robot will automatically sort trash into metal and non-metal bins.
- Telegram notifications will be sent when bins are nearly full.

## Troubleshooting

- Ensure all connections are secure and correct.
- Check serial output for debugging information.
- Verify WiFi connectivity and Telegram bot configuration.
- If motors are not responding, check motor driver connections and power supply.
