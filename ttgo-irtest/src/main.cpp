#include "config.h"
#include <Log.h>
#include <SerialLogger.h>
#include <TVBGone.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>


TVBGone g_tvbgone(TWATCH_2020_IR_PIN);

lv_obj_t *m_imuPoint;

void setup()
{
  Serial.begin(115200);
  Log::init(new SerialLogger());

  // Initialize the hardware, the BMA423 sensor has been initialized internally
  TTGOClass::getWatch()->begin();
  // Turn on the backlight
  TTGOClass::getWatch()->openBL();
  TTGOClass::getWatch()->tft->begin();
  TTGOClass::getWatch()->lvgl_begin();

  // we turn the clock around to have infrared at the top and button on the left
  TTGOClass::getWatch()->lvgl_whirling(4);


  Log::debug("Starting up");
  m_imuPoint = lv_led_create(lv_scr_act(), NULL);
  lv_obj_align(m_imuPoint, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_led_on(m_imuPoint);
  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Arduino! (V7.0.X)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

  delay(50); //50ms (5000x10 us) delay: let everything settle for a bit
}



void loop()
{
  lv_task_handler();
  //Super "ghetto" (but decent enough for this application) button debouncing:
  //-if the user pushes the Trigger button, then wait a while to let the button stop bouncing, then start transmission of all POWER codes

  if (TTGOClass::getWatch()->touched())
  {
    while (TTGOClass::getWatch()->touched())
    {
      delay(10);
    }
    g_tvbgone.sendAllCodes();
  }
  delay(50);
}


