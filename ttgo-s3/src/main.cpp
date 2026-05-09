#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include "ArduinoJson.h"

/*
  LilyGO T-Watch → Home Assistant WebSocket Voice Assistant (Demo)

  What this does
  --------------
  • Connects the ESP32 (T-Watch) to Home Assistant's WebSocket API (/api/websocket)
  • Authenticates with a long‑lived access token
  • Starts an Assist pipeline run (STT→Intent→TTS)
  • Streams microphone audio chunks (16 kHz PCM16 mono) to HA via WS
  • Receives TTS audio chunks back over WS and plays them on the watch speaker via I2S


  Home Assistant setup
  --------------------
  1) In HA: Profile → Security → Create Long‑Lived Access Token → paste into HA_TOKEN below.
  2) Ensure Assist pipeline exists (Settings → Voice Assistants). Use its ID or "default".
*/

#include <WiFi.h>
#include <WebSocketsClient.h>
#include "driver/i2s.h"
#include <vector>
//#include "VoiceApp.h"
#include "config.h"
#include "LoRaApp.h"


// ---------------------- WiFi & WS ----------------------
void connectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

//VoiceApp *voiceApp;
LoRaApp *loraApp;

// ---------------------- Arduino ----------------------
void setup()
{
  // To enable serial print output, you need to enable USB output settings
  // Arduino IDE-> Tools -> USB CDC On Boot -> Enabled
  Serial.begin(115200);

  // Initialize the LilyGoLib instance
  instance.begin();
  delay(200);
  Serial.println("Connecting WiFi...");
  connectWifi();
  Serial.println("WiFi connected.");

  instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
  
  // TODO: do this in the LoraApp
  instance.initLoRa();

  // Call lvgl initialization
  beginLvglHelper(instance);
  loraApp = new LoRaApp();
  loraApp->setup();

  //voiceApp = new VoiceApp();
  //voiceApp->setup();
  

  // Set brightness to MAX
  // T-LoRa-Pager brightness level is 0 ~ 16
  // T-Watch-S3 , T-Watch-S3-Plus , T-Watch-Ultra brightness level is 0 ~ 255
  
}


void loop()
{
  
  //voiceApp->loop();
  loraApp->loop();
  lv_timer_handler();
  // delay(2);
}
