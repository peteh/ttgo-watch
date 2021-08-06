#pragma once

#include "../../config.h"

#include <LilyGoWatch.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Log.h>

#include <mutex>

#include "../AbstractApp.h"
#include "TVBGone.h"

namespace app
{
    class TVBGoneApp : public AbstractApp
    {
    public:
        const static char ID[];
        static TVBGoneApp *instance();

        TVBGoneApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;

        ~TVBGoneApp();

    private:
        static TVBGoneApp *s_instance;
        void updateUI();

        static void _internalEventHandler(lv_obj_t *obj, lv_event_t event)
        {
            if (instance() == nullptr)
            {
                Log::error("Tried to invoke event handler on nullpointer, that should not happen");
                return;
            }

            instance()->buttonEventHandler(obj, event);
        }

        void buttonEventHandler(lv_obj_t *obj, lv_event_t event);

        // Buttons:
        lv_obj_t *m_btnTempUp;
        lv_obj_t *m_rollerFan;

        TVBGone m_tvBGone;
    };
}