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
#include "voicepipeline.h"

// voice pipeline
VoicePipeline vp;

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

static void btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target_obj(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
    Serial.printf("Button :%d\n", cnt);
    vp.startRecording();
  }
}
lv_obj_t *btn;
// ---------------------- Arduino ----------------------
void setup()
{
  // To enable serial print output, you need to enable USB output settings
  // Arduino IDE-> Tools -> USB CDC On Boot -> Enabled
  Serial.begin(115200);

  // Initialize the LilyGoLib instance
  instance.begin();
  delay(200);
  Serial.println("T-Watch ↔ HA WS Voice Demo");
  Serial.println("Connecting WiFi...");
  connectWifi();
  Serial.println("WiFi connected.");

  Serial.println("Setting up Voice Pipe...");
  vp.init();
  Serial.println("Voice Pipe ready.");

  // Call lvgl initialization
  beginLvglHelper(instance);

  lv_obj_t *label = lv_label_create(lv_screen_active()); /*Add a label the current screen*/
  lv_label_set_text(label, "Hello World");               /*Set label text*/
  lv_obj_center(label);                                  /*Set center alignment*/

  btn = lv_button_create(lv_screen_active());                 /*Add a button the current screen*/
  lv_obj_set_size(btn, 120, 50);                              /*Set its size*/
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/
  lv_obj_align_to(btn, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); /*Set the label to it and align it in the center below the label*/

  lv_obj_t *btn_label = lv_label_create(btn); /*Add a label to the button*/
  lv_label_set_text(btn_label, "Button");     /*Set the labels text*/
  lv_obj_center(btn_label);

  // Set brightness to MAX
  // T-LoRa-Pager brightness level is 0 ~ 16
  // T-Watch-S3 , T-Watch-S3-Plus , T-Watch-Ultra brightness level is 0 ~ 255
  instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
}

static bool recording = false;

void loop()
{
  vp.loop();
  if (vp.getState() == VoicePipeline::RECORDING && !recording)
  {
    recording = true;
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (vp.getState() != VoicePipeline::RECORDING && recording)
  {
    recording = false;
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  lv_timer_handler();
  // delay(2);
}
