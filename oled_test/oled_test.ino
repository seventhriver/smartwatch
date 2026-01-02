//  Libraries needed:
//  Time.h & TimeLib.h:  https://github.com/PaulStoffregen/Time
//  Timezone.h: https://github.com/JChristensen/Timezone
//  SSD1306Wire.h:  https://github.com/squix78/esp8266-oled-ssd1306
//  NTPClient.h: https://github.com/arduino-libraries/NTPClient
//  ArduinoJson.h

// 128x64 OLED pinout:
// GND goes to ground
// Vin goes to 3.3V
// Data to I2C SDA (GPIO 4)
// Clk to I2C SCL (GPIO 5)

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <ArduinoJson.h>

// code here

// define button pins
int RightButton = 12;
int MiddleButton = 13;
const int LeftButton = 14;

// Create a display object for the OLED screen
SSD1306Wire  display(0x3C, 4, 5); //0x3C being the usual address of the OLED

// // Define NTP properties
// #define NTP_OFFSET   60 * 60      // In seconds
// #define NTP_INTERVAL 60 * 1000    // In miliseconds
// #define NTP_ADDRESS  "us.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// // Set up the NTP UDP client
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
  // put your setup code here, to run once:
  // open a connection to the serial monitor (param is baud rate)
  Serial.begin(115200); // most ESP-01's use 115200 but this could vary

  Serial.println("Device connected to serial monitor :)");


  Wire.pins(4, 5);  // Start the OLED with GPIO 4 and 5 on ESP-01
  Wire.begin(4, 5); // 4=sda, 5=scl
  display.init();
  display.flipScreenVertically();
  pinMode(LeftButton, INPUT);

  display.drawString(0, 0, "HELLO OLED");
  display.display();
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  int leftButton = digitalRead(LeftButton);
  if (leftButton == LOW) {
    Serial.println("Left button pressed");
    delay(6000);
  }
}