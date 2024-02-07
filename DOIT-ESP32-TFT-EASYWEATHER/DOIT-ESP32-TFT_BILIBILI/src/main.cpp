#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "HarmonyOS_Sans_SC_Regular20.h"

TFT_eSPI tft = TFT_eSPI();
WiFiMulti wifiMulti;
void ArduinoJson_sw(String httpdata);
void display(String tmp, String whr, String wdd, String wdp);
/*解析函数*/
void ArduinoJson_sw(String httpdata)
{
  JsonDocument doc;
  /*解析数据*/
  deserializeJson(doc, httpdata);
  JsonObject obj = doc["lives"][0];
  /*获取数组中的数据并赋值*/
  String province = obj["province"].as<String>();
  String city = obj["city"].as<String>();
  String adcode = obj["adcode"].as<String>();
  String weather = obj["weather"].as<String>();
  String temperature = obj["temperature"].as<String>();
  String winddirection = obj["winddirection"].as<String>();
  String windpower = obj["windpower"].as<String>();
  String humidity = obj["humidity"].as<String>();
  String reporttime = obj["reporttime"].as<String>();

  /*串口显示结果*/
  // Serial.print("[ArduinoJson]");Serial.println(province);delay(100);
  // Serial.print("[ArduinoJson]");Serial.println(city);delay(100);
  // Serial.print("[ArduinoJson]");Serial.println(adcode);delay(100);

  Serial.print("[weather]");
  Serial.println(weather);
  delay(100);
  Serial.print("[temperature]");
  Serial.println(temperature);
  delay(100);
  Serial.print("[winddirection]");
  Serial.println(winddirection);
  delay(100);
  Serial.print("[windpower]");
  Serial.println(windpower);
  delay(100);
  Serial.print("[humidity]");
  Serial.println(humidity);
  delay(100);
  Serial.print("[reporttime]");
  Serial.println(reporttime);
  display(temperature, weather, winddirection, windpower); // 调用显示函数并传递参数
  delay(3000);
}

/*******************************************/
/*显示函数*/

void display(String tmp, String whr, String wdd, String wdp)
{
  tft.fillScreen(TFT_BLACK);                 // 刷新屏幕
  tft.loadFont(HarmonyOS_Sans_SC_Regular20); // 指定tft屏幕对象载入字库
  tft.setCursor(10, 25);                     // 设置位置
  tft.print(tmp);
  tft.println("°C");     // 显示数据
  tft.setCursor(10, 50); // 设置位置
  tft.println(whr);      // 显示数据
  tft.setCursor(10, 75); // 设置位置
  tft.print(wdd);
  tft.print("风");
  tft.setCursor(10, 100); // 设置位置
  tft.print(wdp);
  tft.print("级"); // 显示数据
  delay(1000);
}
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  wifiMulti.addAP("HUAWEI-004V0R", "1977102266");
  tft.init(TFT_BLACK);
  tft.setRotation(0);
}
void loop()
{
  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    // http.begin("https://restapi.amap.com/v3/weather/weatherInfo?city=152921&key=5fed1fcd87dc58a354fd19d50f8b2060&extensions=base", ca); //HTTPS
    http.begin("https://restapi.amap.com/v3/weather/weatherInfo?key=864f1cf5e7070a617712c8a386140fd9&city=370705"); // HTTP
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString(); // 获取返回结果并赋给payload
        Serial.println(payload);           // 串口显示结果
        ArduinoJson_sw(payload);           // 调用解析函数并传递参数
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  delay(5000);
}
