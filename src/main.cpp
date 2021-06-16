


/* FastLED RGBW Example Sketch
 *
 * Example sketch using FastLED for RGBW strips (SK6812). Includes
 * color wipes and rainbow pattern.
 *
 * Written by David Madison
 * http://partsnotincluded.com
*/

//#include "FastLED.h"

//#include "GlowStrip.h"
#define FASTLED_ESP32_I2S true

#include "FastLEDGlowStrip.h"
//#include "GlowBehaviours.h"
#include "GlowController.h"
//#include "ArduinoJson.h"
#include "FastLED_RGBW.h"
#include "GlowMQTT.h"
#include "NTPFeature.h"
#include "WiFIFeature.h"
#include "SerialConnector.h"

//Need to include this here if we want it in the WiFiFeature
#include <ArduinoOTA.h>


#ifndef NUM_LEDS
#define NUM_LEDS 100
#endif

#ifndef LED_DATA_PIN
#define LED_DATA_PIN 12
#endif

#define LEDS_PER_STRIP 400
#define NUM_STRIPS 3

//#define STATE_LOGGING 1
//#define TIME_LOGGING 1

/* WIFI Defs */
const char* ssid = "Fishbowl2G";
const char* password = "29Goldfish";

/* MQTT Defs */
const char* mqtt_server = "192.168.178.108";
const int mqtt_port = 1883;

/* NTP Defs */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* id = "living";
const char* name = "Living Room Lights";

#define str(x) #x
#define strname(name) str(name)

#ifndef DEVICE_ID
#define DEVICE_ID test
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME Test Device
#endif

WiFiClient esp;
PubSubClient client(esp);

FRGBW current[NUM_LEDS];
CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

const uint8_t brightness = 255;

FastLEDGlowStrip strip(NUM_LEDS,leds, current);
//GlowController glowControl(&strip,id,name);
GlowController glowControl(&strip,strname(DEVICE_ID),strname(DEVICE_NAME));

bool ota_running = false;

/*
  {
    "id":2,
    "type":"PixelClock",
    "name":"Time"
  },
  {
    "id":5,
    "type":"Glow",
    "name":"Red",
    "data":{"r":0.8,"b":0.0,"g":0.0,"w":0.0,"rate":0.4}
  },
*/

static const char fullJson[] PROGMEM = ( R"(
{
  "behaviours":
  [
    {
      "id":0,
      "type":"Fill",
      "name":"Base",
      "data":{"r":0.0,"b":0.0,"g":0.0,"w":0.4}
    },


    {
      "id":6,
      "type":"Alarm",
      "name":"Alarms",
      "data":{"start":0.01,"length":10,"time":10}
    },
    {
      "id":16,
      "type":"Watchdog",
      "name":"Chk"
    }
  ]
})");

/*

*/






void setup() {
  glowControl.timeKeeping()->logState = 0;
  glowControl.timeKeeping()->logTime = 0;


  Serial.begin(115200);
  while(!Serial && millis() < 4000 ) {}
  Serial.println("Starting out...");

  /* Serial version */
  FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  /* Parallel version */
  //FastLED.addLeds<WS2812B, 12, RGB>(ledsRGB, getRGBWsize(LEDS_PER_STRIP));
  //FastLED.addLeds<WS2812B, 14, RGB>(ledsRGB + 4 * LEDS_PER_STRIP, getRGBWsize(LEDS_PER_STRIP));
  //FastLED.addLeds<WS2812B, 15, RGB>(ledsRGB + 8 * LEDS_PER_STRIP, getRGBWsize(LEDS_PER_STRIP));

  FastLED.setBrightness(brightness);
  strip.show();


  glowControl.addFeature(new WiFiFeature(ssid,password));
  glowControl.addFeature(new NTPFeature(ntpServer,gmtOffset_sec,daylightOffset_sec));
  glowControl.addConnector(new SerialConnector());
  glowControl.addConnector(new MQTTConnector(&client, mqtt_server, mqtt_port));

  glowControl.update(fullJson);
  Serial.println(F("Done loading behaviours"));

  Serial.println(F("Starting up"));
  FRGBW startupColor = FRGBW(0,0,0,0.5);
  Fill* base = (Fill*)glowControl.getBehaviour(0);
  base->fadeIn(startupColor,16);
  base->setInterpTime(.2);
  //setupWiFi(ssid,password);
  //glowControl.sendState();

}

void loop(){
  glowControl.loop();
  /*
  if( ! ota_running && WiFi.status() == WL_CONNECTED ) {
    ota_running = true;
    Serial.println("Starting OTA");
    ArduinoOTA.setHostname(strname(DEVICE_ID));
    ArduinoOTA.begin();
  }
  if(ota_running ) { ArduinoOTA.handle();}
  */
}
