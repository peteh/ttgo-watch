#pragma once
#include "AbstractApp.h"
#include <PubSubClient.h>
#include <LilyGoWatch.h>
#include "../Remote.h"

namespace app
{
    class RemoteControlApp : public AbstractApp
    {
    public:
        static const char ID[];
        RemoteControlApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;

    private:
        TFT_eSPI *m_tft;
        BMA *m_bmaSensor;
        Remote m_remote;
        uint8_t m_prevRotation;

        float m_left;
        float m_right;

        long m_timestampLastSend;

        lv_obj_t * m_imuPoint;
    };
}