
#define FASTLED_ESP32_I2S true

#include <strip/FastLEDGlowStrip.h>
#include <ControlDefaults.h>
#include <GlowController.h>
#include <strip/FastLED_RGBW.h>

#include <ArduinoOTA.h>

//#define STATE_LOGGING 1
//#define TIME_LOGGING 1


FRGBW current[NUM_LEDS];
CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

const uint8_t brightness = 255;

FastLEDGlowStrip strip(NUM_LEDS,leds, current);
GlowController glowControl(&strip,strname(DEVICE_ID),strname(DEVICE_NAME));


void setup() {
    //Serial.begin(115200);
    //Serial.println("Starting");
    FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));

    FastLED.setBrightness(brightness);
    strip.fillRGBW(FRGBW(0,0,0,0),0,NUM_LEDS-1);
    strip.show();
    glowControl.initialise();
    glowControl.setupBaseFeatures(strname(WIFI_SSID), strname(WIFI_PASSWORD), strname(MQTT_SERVER), MQTT_PORT, GMT_OFFSET, DAYLIGHT_OFFSET, strname(NTP_SERVER));
    glowControl.setupControls();
    glowControl.setupBehaviours();
    glowControl.loadState();

}

void loop() {
    glowControl.loop();
}