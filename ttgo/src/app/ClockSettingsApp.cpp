#include "ClockSettingsApp.h"
#include <Log.h>
namespace app
{
    const char ClockSettingsApp::ID[] = "app.clocksettings";

    // TODO: proper destructor
    ClockSettingsApp *ClockSettingsApp::s_instance = nullptr;

    ClockSettingsApp::ClockSettingsApp()
        : AbstractApp()
    {
        // HACK: for the event listeners as they can only be static functions
        s_instance = this;
    }

    ClockSettingsApp::~ClockSettingsApp()
    {
        Log::debug("destructor 1");
        s_instance = nullptr;

        // TODO: rather use parents to delete children in lvgl
        // Buttons:

        lv_obj_del(m_btnSyncNTP);
        lv_obj_del(m_rollerFan);
    }

    ClockSettingsApp *ClockSettingsApp::instance()
    {
        return s_instance;
    }

    void ClockSettingsApp::buttonEventHandler(lv_obj_t *obj, lv_event_t event)
    {
        if (obj == m_btnSyncNTP && event == LV_EVENT_CLICKED)
        {
            Log::debug("Starting NTP Sync");
            const char *ntpServer = "pool.ntp.org";
            const char *ntpServer2 = "cn.pool.ntp.org";
            const long gmtOffset_sec = 8 * 60 * 60;
            const int daylightOffset_sec = 0;
            //init and get the time
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2);

            Log::debug("Finished NTP Sync");
            getWatch()->rtc->syncToRtc();
        }

        if (obj == m_rollerFan && event == LV_EVENT_VALUE_CHANGED)
        {
        }
    }

    void ClockSettingsApp::updateUI()
    {
    }

    void ClockSettingsApp::setupApp()
    {

        // Turn on the backlight
        getWatch()->openBL();
        m_btnSyncNTP = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnSyncNTP, _internalEventHandler);
        lv_obj_align(m_btnSyncNTP, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_btn_set_fit2(m_btnSyncNTP, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t *labelTempUp = lv_label_create(m_btnSyncNTP, NULL);
        lv_label_set_text(labelTempUp, "Start");

        m_rollerFan = lv_roller_create(lv_scr_act(), NULL);
        lv_roller_set_options(m_rollerFan,
                              "EU\n"
                              "NA",
                              LV_ROLLER_MODE_NORMAL);

        lv_roller_set_visible_row_count(m_rollerFan, 3);
        lv_obj_align(m_rollerFan, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        lv_obj_set_event_cb(m_rollerFan, _internalEventHandler);

        // force first update
        lv_scr_load(lv_scr_act());
        updateUI();
    }

    const char *ClockSettingsApp::loopApp()
    {
        lv_task_handler();
        delay(5);
        return nullptr;
    }
}