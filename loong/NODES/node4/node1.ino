/******************************
 * Code for Node 1            *
 * Author: Radioactive-jkl    *
 * Date: 2024/06/26           *
 ******************************/

 
/*
 * Includes
 */
#include <WiFi.h>
#include <math.h>
#include <Wire.h>
#include <BMP280.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <AliyunIoTSDK.h>
#include <Adafruit_AHTX0.h>
#include "PubSubClient.h"

/*
 * User Definition
 */
// WIFI
#define WIFI_SSID     "RADIOACTIVE"
#define WIFI_PASSWD   "15172272767"
static WiFiClient espClient;

// Aliyun-IoT
#define PRODUCT_KEY   "jk3pVO4Uxky"
#define DEVICE_NAME   "node1"
#define DEVICE_SECRET "ca77c33d4ed9d6a2206a2419b0969813"
#define REGION_ID     "cn-shanghai"

/*
 * Hardware Definition
 */
// OLED
#define sda_oled 5
#define scl_oled 23
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, scl_oled, sda_oled, U8X8_PIN_NONE);

// AHT
#define sda_aht  21
#define scl_aht  22
BMP280 bmp280;
Adafruit_AHTX0 aht;

// MQ2
#define D0 33
#define A0 32
static float R0;

// BAT
#define ANALOG_PIN 34
int percentage = 100;
double capacity[101]={
          4.179,4.169,4.157,4.146,4.134,4.123,4.111,4.099,4.088,4.076,
          4.063,4.051,4.040,4.028,4.017,4.005,3.994,3.982,3.971,3.960,
          3.949,3.938,3.927,3.916,3.906,3.895,3.885,3.875,3.865,3.855,
          3.845,3.836,3.827,3.818,3.809,3.800,3.792,3.784,3.775,3.768,
          3.760,3.753,3.745,3.738,3.732,3.725,3.719,3.713,3.707,3.701,
          3.695,3.690,3.685,3.679,3.675,3.670,3.665,3.661,3.657,3.652,
          3.648,3.645,3.641,3.637,3.633,3.630,3.626,3.623,3.619,3.616,
          3.613,3.609,3.606,3.602,3.599,3.595,3.592,3.588,3.584,3.580,
          3.576,3.571,3.567,3.562,3.557,3.552,3.546,3.540,3.534,3.528,
          3.521,3.513,3.506,3.498,3.489,3.480,3.370,3.260,3.149,3.138,
          3.126};/*Points on curve of V-SOC | 1S(3.7v)lithium battery 20℃*/


/*
 * Tool Functions
 */
// Tools for WiFi
void setupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0,30);
  u8g2.print("Connecting");
  u8g2.sendBuffer();
  while (!WiFi.isConnected())
  {
    u8g2.print(".");
    u8g2.sendBuffer();
    delay(500);
  }
  u8g2.clearBuffer();
  u8g2.setCursor(0,20);
  u8g2.print("Wifi connected!");
  u8g2.setCursor(0,40);
  u8g2.print("IP address: ");
  u8g2.setCursor(0,60);
  u8g2.print(WiFi.localIP());
  u8g2.sendBuffer();
  delay(2000);
}

/*
 * Data Structure
 */
struct AHT_values
{
  float temperature;
  float humidity;
  int   pressure;
};

struct MQ2_values
{
  float smokescope;
};

struct BAT_values
{
  int battery;
};

struct Values
{
  struct AHT_values ahtValues;
  struct MQ2_values mq2Values;
  struct BAT_values batValues;
};


/*
 * Functions for Structure
 */
// Aliyun-MQTT
void MQTT_write(struct Values temp)
{
  AliyunIoTSDK::send((char*)"temperature1",    temp.ahtValues.temperature);
  AliyunIoTSDK::send((char*)"humidity1",       temp.ahtValues.humidity);
  AliyunIoTSDK::send((char*)"pressure1",       temp.ahtValues.pressure);
  AliyunIoTSDK::send((char*)"mokescope1",      temp.mq2Values.smokescope);
  AliyunIoTSDK::send((char*)"battery1",        temp.batValues.battery);
}

// OLED
void OLED_write(struct Values temp)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.setCursor(0,10);
  u8g2.print("Node 1:");
  
  u8g2.setCursor(0,20);
  u8g2.print("Tempera: ");
  u8g2.setCursor(64,20);
  u8g2.print(String(temp.ahtValues.temperature)+" *C");
  
  u8g2.setCursor(0,30);
  u8g2.print("Humidit: ");
  u8g2.setCursor(64,30);
  u8g2.print(String(temp.ahtValues.humidity)+" %RH");

  u8g2.setCursor(0,40);
  u8g2.print("Pressur: ");
  u8g2.setCursor(64,40);
  u8g2.print(String(temp.ahtValues.pressure)+" Pa");
  
  u8g2.setCursor(0,50);
  u8g2.print("Smokesc: ");
  u8g2.setCursor(64,50);
  u8g2.print(String(temp.mq2Values.smokescope)+" ppm");

  u8g2.setCursor(0,60);
  u8g2.print("Battery: ");
  u8g2.setCursor(64,60);
  u8g2.print(String(temp.batValues.battery)+" %");
  
  u8g2.sendBuffer();
}

// AHT
struct AHT_values AHT_read()
{
  struct AHT_values temp = {0.0, 0.0, 0};
  temp.pressure = bmp280.getPressure();
  sensors_event_t humidity, temperature;
  aht.getEvent(&humidity, &temperature);
  temp.temperature = temperature.temperature;
  temp.humidity = humidity.relative_humidity;
  return temp;
}

// MQ2
struct MQ2_values MQ2_read()
{
  struct MQ2_values temp = {0.0};
  // calibrate
  if(millis() < 60000)
  {
    float Vrl = 3.3f * analogRead(A0) / 4096.f;
    float RS = (3.3f - Vrl) / Vrl * 5.00;
    R0 = RS / pow(20 / 613.9f, 1 / -2.074f); 
  }
  // read adc
  float adc = 0.0;
  for (int i = 0; i < 10; i++)
  {
    adc += analogRead(A0);
    delay(5);
  }
  adc = adc/10.0;
  // calculate
  float Vrl = 3.3f * adc / 4096.f;
  float RS = (3.3 - Vrl) / Vrl * 5.00;
  float ppm = 613.9 * pow(RS/R0, -2.074);
  temp.smokescope = ppm;
  return temp;
}

// BAT
struct BAT_values BAT_read()
{
  struct BAT_values temp = {0};
  float val=0;
  for (int i = 0; i < 10; i++)
  {
    val+=analogRead(ANALOG_PIN);
    delay(5);
  }
  val /= 10;
  float voltage = (((float)val)/4095)*3.3*3.0;
  for (int i = 0; i < 100; i++)
  {
    if ((voltage>=capacity[i+1]) || (voltage<=capacity[i]))
    {
      percentage = 100 - i;
      break;
    }
  }
  temp.battery = percentage;
  return temp;
}

/*
 * Main Function
 */
void setup()
{
  //LED
  pinMode(2, OUTPUT);

  // OLED
  u8g2.begin();

  // AHT
  Wire.begin(sda_aht,scl_aht); 
  bmp280.begin();
  aht.begin();

  // WiFi
  setupWifi();

  // AliyunIoT
  AliyunIoTSDK::begin(espClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);
}

void loop()
{
  // Check WiFi
  while (!WiFi.isConnected())
  {
    setupWifi();
  }
  
  struct Values sensorValues = {{0.0, 0.0, 0}, {0.0}, {0}};
  
  sensorValues.ahtValues = AHT_read();
  sensorValues.mq2Values = MQ2_read();
  sensorValues.batValues = BAT_read();
  
  OLED_write(sensorValues);
  MQTT_write(sensorValues);
  
  AliyunIoTSDK::loop();
}