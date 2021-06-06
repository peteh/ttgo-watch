#pragma once

#define LILYGO_WATCH_2020_V3 //To use T-Watch2020 V3, please uncomment this line
// TODO: find a way to do that for all

#include <LilyGoWatch.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

#include "AbstractApp.h"

namespace app
{
    class ACControl : public AbstractApp
    {
    public:
        virtual void setup() override;
        virtual void loop() override;

    private:
        TTGOClass *m_watch;
        TFT_eSPI *m_tft;
        IRGreeAC *m_ac;

        bool m_on;
        uint8_t m_pin;
    };
}