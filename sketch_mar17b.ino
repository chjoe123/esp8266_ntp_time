/*
 * This program work on ESP8266 Standard library.
 * 
 * The screen chip is ST7735S, you can find the library from github - "TFT_eSPI.h"
 * Arduinojson library 5 work fine for me.
 */

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

const char* ssid  = "XX";
const char* pass  = "XX";
String apiKey = "XX";  //your api key
String cityId = "WQJ6YY8MHZP0"; //change for your city id, you can find it on document 
const char* serverName = "api.seniverse.com";
String weat;
int temp;
unsigned long pastTime = 0;
unsigned long currentTime;
byte omm = 99;
WiFiClient client;

const long utcOffsetInSeconds = 28800;    //UTC +8.00
char dayOfTheWeek[7][12]  = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 0, 2);
  tft.print("Connecting WiFi >>>"); tft.println(ssid);
  
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  timeClient.begin();
}

void loop() {
  // put your main code here, to run repeatedly

  /**
  Serial.print(dayOfTheWeek[timeClient.getDay()]);    //day
  Serial.print(", ");
  Serial.print(timeClient.getHours());  //Hours
  Serial.print(":");
  Serial.print(timeClient.getMinutes());  //Minutes
  Serial.print(":");
  Serial.print(timeClient.getSeconds());  //Seconds
  Serial.println();
  */
  currentTime = millis();
  if ( ((currentTime-pastTime) >= 60000) || (pastTime == 0) )    {
    getWeatherData();
    pastTime = currentTime;
  }
  
  timeClient.update();

  char buf[50];
  sprintf(buf, "%02d:%02d", 
          timeClient.getHours(), timeClient.getMinutes());
  byte mm = timeClient.getMinutes();
  if ( omm != mm )   {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(10, 0, 6);
    tft.println(buf);
    tft.setCursor(60,40,2);
    tft.print(dayOfTheWeek[timeClient.getDay()]);
    tft.setCursor(20,60,2);
    tft.print(weat);
    tft.print("    ");
    tft.print(temp); tft.println(" C");
    omm = mm;
  }
  delay(1000);
}

void getWeatherData()   {
  //https://api.seniverse.com/v3/weather/now.json?key=SvapyYFkj7EvAAqfG&location=beijing&language=zh-Hans&unit=c
  if ( client.connect(serverName, 80) ) {
    client.println("GET /v3/weather/now.json?key="+apiKey+"&language=en&location="+cityId);
    client.println("Host: api.seniverse.com");
    client.println("Connection: close");
    client.println();
  }

  String result = "";
  while (client.connected() && !client.available()) delay(1); 
  while (client.connected() || client.available()) { 
    char c = client.read();
    result = result + c;
  }
  client.stop();
  char jsonArray[result.length()+1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length()+1] = '\0';
  Serial.println();
  Serial.println(jsonArray);
  StaticJsonBuffer<1024> json_buf;
  JsonObject &root = json_buf.parseObject(jsonArray);
  String weather = root["results"][0]["now"]["text"];
  int temperature = root["results"][0]["now"]["temperature"];
  weat = weather; 
  temp = temperature;
}
