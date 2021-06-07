


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

#include "FastLEDGlowStrip.h"
//#include "GlowBehaviours.h"
#include "GlowController.h"
//#include "ArduinoJson.h"
#include "FastLED_RGBW.h"
#include "GlowMQTT.h"



#define NUM_LEDS 1200
#define DATA_PIN 2

//#define STATE_LOGGING 1
//#define TIME_LOGGING 1

const char* ssid = "Fishbowl2G";
const char* password = "29Goldfish";

const char* mqtt_server = "192.168.178.123";
const int mqtt_port = 1883;




FRGBW current[NUM_LEDS];
CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

const uint8_t brightness = 255;

FastLEDGlowStrip strip(NUM_LEDS,leds, current);


//Fill base(&strip,FRGBW(0,0,0,0.0));

float start = 9.0 / NUM_LEDS;
float width = 9.0 / NUM_LEDS;
float rate = 4.0 / NUM_LEDS;
//Glow ball(&strip,AreaRGBW(FRGBW(1,0,0,0.0),start,width),rate);
//Glow ball2(&strip,AreaRGBW(FRGBW(0,0,0,1.0),start+0.2,width),rate*1.1);
//Watchdog watchdog(&strip,8);

GlowController glowControl(&strip,true);


float r = 0;

long lastUpdate = millis();



//char json[] =
      //"{\"update\":0,\"data\":{\"r\":0.5,\"g\":0.1,\"b\":0.3,\"w\":0.7}}";
//char baseJson[] =
  //"{\"id\":0, \"type\":\"Fill\", \"name\":\"Background\", \"data\":{\"r\":0.5, \"g\":0.1, \"b\":0.3, \"w\":0.7} }";
//

static const char fullJson[] PROGMEM = ( R"(
{
  "behaviours":
  [
    {
      "id":0,
      "type":"Fill",
      "name":"Background",
      "data":{"r":0.0,"b":0.0,"g":0.0,"w":0.4}
    },
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
    {
      "id":16,
      "type":"Watchdog",
      "name":"Watchdog"
    }
  ]
})");

/*

*/






void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 4000 ) {}
  Serial.println("Starting out...");

  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.setBrightness(brightness);
  strip.show();

  setupGlowMQTT(&glowControl, ssid, password, mqtt_server, mqtt_port);

  glowControl.update(fullJson);
  Serial.println(F("Done loading behaviours"));

  Serial.println(F("Starting up"));
  FRGBW startupColor = FRGBW(0,0,0,0.5);
  Fill* base = (Fill*)glowControl.getBehaviour(0);
  base->fadeIn(startupColor,16);
  base->setInterpTime(.2);
  glowControl.sendState();
}

void loop(){
  #ifdef STATE_LOGGING
  Serial.println("Loop...");
  #endif

  long t = millis();
  mqttConnect();
  glowControl.update();


  #ifdef STATE_LOGGING
  Serial.println("About to start behaviours loop...");
  #endif

  glowControl.runBehaviours();

  #ifdef STATE_LOGGING
  Serial.println("Done behaviours");
  #endif

  long update_t = millis() - t;
  #ifdef TIME_LOGGING
    Serial.print("Update time: ");Serial.print(update_t);
    Serial.print(" FPS: ");Serial.println((int)(1000.0/update_t));
  #endif
}
