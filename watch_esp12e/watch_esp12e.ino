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

//Also need to download driver for usb-uart board

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <ArduinoJson.h>

const int LeftButton = 13;
const int RightButton = 14;
const int MiddleButton = 12;

// Define NTP properties
#define NTP_OFFSET   0     // In seconds - no offset here, we correct for timezone later
#define NTP_INTERVAL 60 * 1000    // In miliseconds - frequency of syncing time with NTP
#define NTP_ADDRESS  "us.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Create a display object
SSD1306Wire  display(0x3C, 4, 5); //0x3C is the usual hardware address of the OLED

const char* ssid = "{wifi-network-name}";   // insert your own ssid
const char* password = "{wifi-network-password}";         // and password
String date;
String t;
String temp;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

const String url = "https://api.openweathermap.org/data/2.5/weather?"; //put the link to Weather API here
const String ApiKey = "7ca95e3c3f554337c5ff41944d56ae2f";

// Replace with your location
const String lat = "{latitude}";
const String lon = "{longitude}";

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // most ESP-01's use 115200 but this could vary
  timeClient.begin();   // Start the NTP UDP client

  Wire.begin(4, 5); // 4=sda, 5=scl
  display.init();
  display.flipScreenVertically();
  pinMode(LeftButton, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  display.drawString(0, 0, "Connected to WiFi.");
  display.drawString(0, 24, "Hi {Name}!");
  display.display();
  delay(1000);
}

void loop() {
  int buttonState = digitalRead(LeftButton);
  if (buttonState == LOW) {
    Serial.print("Button pressed");
    GetWeatherData();
    tellTime();
    delay(6000);
  }

  display.display();
}

void tellTime() {
  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {
    date = "";  // clear the variables
    t = "";

    // update the NTP client and get the UNIX UTC timestamp
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
    TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
    Timezone usCentral(usCDT, usCST);
    local = usCentral.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local) - 1];
    date += ", ";
    date += months[month(local) - 1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    if (minute(local) < 10) // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

    // Display the date and time
    Serial.println("");
    Serial.print("Local date: ");
    Serial.print(date);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);

    // print the date and time on the OLED
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawStringMaxWidth(64, 14, 128, t); // print time on the display
    display.setFont(ArialMT_Plain_10);
    display.drawStringMaxWidth(64, 42, 128, date); // print date on the display
    
    display.drawString(70, 0, "Temp:"); // prints the Temperature from GetWeatherData() function
    display.drawString(100, 0, temp); 
    display.drawString(113, 0, "F");
    display.display();
  }
  else // attempt to connect to wifi again if disconnected
  {
    display.clear();
    display.drawString(0, 18, "Connecting to Wifi...");
    display.display();
    WiFi.begin(ssid, password);
    display.drawString(0, 32, "Connected.");
    display.display();
  }
}

void GetWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();    
    String request = url + "lat=" + lat + "&lon=" + lon + "&units=imperial&appid=" + ApiKey;
    Serial.print("Sending HTTP request to ");
    Serial.println(request);
    http.begin(client, request); 
    int httpCode = http.GET();
    delay(500);

    if (httpCode == 200) {
      //Read Data as a JSON string
      String JSON_Data = http.getString();
      Serial.println(JSON_Data);

      //Retrieve some information about the weather from the JSON format
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, JSON_Data);
      JsonObject obj = doc.as<JsonObject>();

      //Retrieve the Current Weather Info
      const char* description = obj["weather"][0]["description"].as<const char*>();
      const float tempF = obj["main"]["temp"].as<float>();
      temp = String(tempF, 1);

    } else {
      Serial.print("Weather - API request failed with code ");
      Serial.println(httpCode);
    }

    http.end();
  }
}
