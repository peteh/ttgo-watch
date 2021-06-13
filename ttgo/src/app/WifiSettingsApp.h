#pragma once

#include "AbstractApp.h"
#include <LilyGoWatch.h>
#include <Thread.h>

namespace app
{
    class WifiSettingsApp : public AbstractApp
    {
    public:
        static const char ID[];
        static WifiSettingsApp *instance();

        WifiSettingsApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;
        virtual ~WifiSettingsApp();

    private:
        static WifiSettingsApp *s_instance;

        static void _internalEventHandler(lv_obj_t *obj, lv_event_t event)
        {
            instance()->buttonEventHandler(obj, event);
        }

        void buttonEventHandler(lv_obj_t *obj, lv_event_t event);
        void createKeyboard();
        void scanWifi();

        lv_obj_t *m_btnScan;
        lv_obj_t *m_btnConnect;
        lv_obj_t *m_rollerWifiSSID;
        lv_obj_t *m_textPassword;
        lv_obj_t *m_keyboard;
    };
}