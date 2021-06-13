#pragma once

#include "../config.h"

#include <LilyGoWatch.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Kelvinator.h>
#include <ir_Gree.h>
#include <Log.h>

#include <mutex>

#include "AbstractApp.h"

namespace app
{
    class ACControlApp : public AbstractApp
    {
    public:
        const static char ID[];
        static ACControlApp *instance();

        ACControlApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;
        virtual void tearDownApp() override;

        ~ACControlApp();

    private:
        static ACControlApp *s_instance;
        void setTemp(uint8_t targetTemp);
        void updateUI();
        void loadFromConfig();
        void saveToConfig();

        uint8_t getModeFromSelection(uint index);
        uint8_t getSelectionFromMode(uint mode);

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

        bool m_configChanged;
        long m_timestampLastSave;

        std::mutex m_mutexFile;

        TTGOClass *m_watch;
        IRGreeAC *m_ac;

        // Buttons:
        lv_obj_t *m_btnTempUp;

        lv_obj_t *m_btnTempDown;

        lv_obj_t *m_switchPower;
        lv_obj_t *m_labelPower;

        lv_obj_t *m_switchTurbo;
        lv_obj_t *m_labelTurbo;

        lv_obj_t *m_switchSwing;
        lv_obj_t *m_labelSwing;

        lv_obj_t *m_dropdownMode;

        lv_obj_t *m_labelTemp;

        lv_obj_t *m_rollerFan;
    };
}