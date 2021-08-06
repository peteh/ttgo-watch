#pragma once

#include "../config.h"

#include <LilyGoWatch.h>
#include <Log.h>

#include "AbstractApp.h"
#include "ClockSettingsApp.h"

namespace app
{
    class ClockSettingsApp : public AbstractApp
    {
    public:
        const static char ID[];
        static ClockSettingsApp *instance();

        ClockSettingsApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;

        ~ClockSettingsApp();

    private:
        static ClockSettingsApp *s_instance;
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
        lv_obj_t *m_btnSyncNTP;
        lv_obj_t *m_rollerFan;
    };
}