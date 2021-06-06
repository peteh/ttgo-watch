#pragma once

#define LILYGO_WATCH_2020_V3 //To use T-Watch2020 V3, please uncomment this line
// TODO: find a way to do that for all

#include "AbstractApp.h"
#include <PubSubClient.h>
#include <LilyGoWatch.h>
#include "../Remote.h"

namespace app
{
    class RemoteControl : public AbstractApp
    {
    public:
        virtual void setup() override;
        virtual void loop() override;

    private:
        TTGOClass *m_watch;
        TFT_eSPI *m_tft;
        BMA *m_bmaSensor;
        Remote m_remote;
        uint8_t m_prevRotation;

        float m_left;
        float m_right;
    };
}