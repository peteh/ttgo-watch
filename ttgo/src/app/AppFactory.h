#pragma once
#include <Arduino.h>
#include <Log.h>
#include "App.h"

#include "ACControlApp.h"
#include "ClockDigitalApp.h"
#include "ClockStarTrekApp.h"
#include "ClockSettingsApp.h"
#include "MainMenuApp.h"
#include "RemoteControlApp.h"
#include "WifiSettingsApp.h"
#include "TVBGone/TVBGoneApp.h"

namespace app
{
    class AppFactory
    {
    public:
        IApp *createApp(const char *appId)
        {
            // TODO: make this generic
            if (strcmp(appId, ACControlApp::ID) == 0)
            {
                return new ACControlApp();
            }

            if (strcmp(appId, ClockDigitalApp::ID) == 0)
            {
                return new ClockDigitalApp();
            }

            if (strcmp(appId, ClockSettingsApp::ID) == 0)
            {
                return new ClockSettingsApp();
            }

            if (strcmp(appId, ClockStarTrekApp::ID) == 0)
            {
                return new ClockStarTrekApp();
            }

            if (strcmp(appId, MainMenuApp::ID) == 0)
            {
                return new MainMenuApp();
            }

            if (strcmp(appId, RemoteControlApp::ID) == 0)
            {
                return new RemoteControlApp();
            }

            if (strcmp(appId, TVBGoneApp::ID) == 0)
            {
                return new TVBGoneApp();
            }

            if (strcmp(appId, WifiSettingsApp::ID) == 0)
            {
                return new WifiSettingsApp();
            }

            Log::errorf("App with ID %s does not exist, will die now", appId);
            return nullptr;
        }
    };
}