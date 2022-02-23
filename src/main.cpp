


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
#include "ControlManager.h"

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

//const char* id = "living";
//const char* name = "Living Room Lights";

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

DynamicJsonDocument controlsSetup(2000);

#include "SPIFFS.h" 

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
      "data":{"r":0.1,"b":0.0,"g":0.0,"w":0.3}
    },
    {
      "id":14,
      "type":"Watchdog",
      "name":"Watchdog"
    }
  ]
})");

/*
    {
      "id":4,
      "type":"Glow",
      "name":"Glow 1",
      "data":{"r":0.8,"b":0.0,"g":0.0,"w":0.0,"rate":0.4}
    },
    {
      "id":5,
      "type":"Glow",
      "name":"Glow 2",
      "data":{"r":0.8,"b":0.0,"g":0.0,"w":0.0,"rate":0.4}
    },

*/

static const char controllerJSON[] PROGMEM = ( R"(
{
  "controls":
  [
    {
      "type":"knob",
      "id":1,
      "pin":5,
      "path":["data","r"]
    },
    {
      "type":"toggle",
      "id":0,
      "pin":27,
      "path":["active"]
    },
    {
      "type":"cknob",
      "id":0,
      "pin":34,
      "max":2,
      "min":-2,
      "deadzone":0.5,
      "path":["data","h+"]
    },
  
    {
      "type":"value",
      "id":0,
      "pin":4,
      "value":20,
      "path":["data","h+"]
    }
  ]
}
)");




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

  DeserializationError error = deserializeJson(controlsSetup, controllerJSON);
  Serial.println("**** Setting up Controls ****");
  if (error) { 
    Serial.println("!! No controls");
    Serial.print(F("deserializeJson() failed for controls: ")); Serial.println(error.f_str()); } 
  else {
    Serial.println("Making controls:");
    glowControl.addFeature(new Controllers(controlsSetup.as<JsonVariant>()));
  }
  Serial.println("**** Done Setting up Controls ****");

  Serial.println(F("-----\nLoading behaviours\n-------"));
  DynamicJsonDocument initialState(8000);
  bool initialisedFromFile = false;
  if( SPIFFS.begin() ) {
    Serial.println("Filesystem OK");
    if( SPIFFS.exists("/conf.json") ) { 
      Serial.println("Config file exists");
    }
    else { Serial.println("File not found...");}
    //File f = SPIFFS.open('/in.json',"r");
    File f = SPIFFS.open("/conf.json", "r");
    if( f ) {
      DeserializationError error = deserializeJson(initialState, f);
      if (error) { Serial.print(F("deserializeJson() failed: ")); Serial.println(error.f_str()); } 
      else { initialisedFromFile = true; }
    }
  } else { Serial.println("Couldn't load file"); }

  if( ! initialisedFromFile ) {
    DeserializationError error = deserializeJson(initialState, fullJson);
    if (error) { Serial.print(F("deserializeJson() failed: ")); Serial.println(error.f_str()); }
  }


  glowControl.processInput(initialState.as<JsonVariant>());
  Serial.println(F("-----\nDone Loading behaviours\n-------"));

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
}
