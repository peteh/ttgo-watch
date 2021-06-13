#include "config.h"

#include <Arduino.h>
#include <LilyGoWatch.h>

#include <app/ACControlApp.h>
#include <app/MainMenuApp.h>
#include <app/WifiSettingsApp.h>
#include <app/AppFactory.h>
#include <app/ButtonTest.h>

#include <PubSubClient.h>

#include <Log.h>
#include <SerialLogger.h>

app::IApp *g_app;
app::AppFactory g_appFactory;

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

    Log::debug("Mounting FS...");
    if (!SPIFFS.begin())
    {
        Log::error("Failed to mount file system");
        return;
    }

    // we turn the clock around to have infrared at the top and button on the left
    TTGOClass::getWatch()->lvgl_whirling(4);
    //g_app = g_appFactory.createApp(app::MainMenuApp::ID);
    g_app = g_appFactory.createApp(app::WifiSettingsApp::ID);
    g_app->setup();
}

void loop()
{
    const char *nextAppID = g_app->loop();

    if (nextAppID != nullptr)
    {
        app::IApp *currentApp = g_app;
        Log::infof("Main: Switching to %s", nextAppID);
        currentApp->tearDown();
        delete (currentApp);
        
        g_app = g_appFactory.createApp(nextAppID);
        g_app->setup();
        
    }
}