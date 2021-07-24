#include "TVBGoneApp.h"
#include <Log.h>
namespace app
{
    const char TVBGoneApp::ID[] = "app.tvbgone";

    // TODO: proper destructor
    TVBGoneApp *TVBGoneApp::s_instance = nullptr;

    TVBGoneApp::TVBGoneApp()
        : AbstractApp(),
          m_tvBGone(TWATCH_2020_IR_PIN)
    {
        // HACK: for the event listeners as they can only be static functions
        s_instance = this;
    }

    TVBGoneApp::~TVBGoneApp()
    {
        Log::debug("destructor 1");
        s_instance = nullptr;

        // TODO: rather use parents to delete children in lvgl
        // Buttons:

        lv_obj_del(m_btnTempUp);
        lv_obj_del(m_rollerFan);
    }

    TVBGoneApp *TVBGoneApp::instance()
    {
        return s_instance;
    }

    void TVBGoneApp::buttonEventHandler(lv_obj_t *obj, lv_event_t event)
    {
        if (obj == m_btnTempUp && event == LV_EVENT_CLICKED)
        {
            m_tvBGone.sendAllCodes();
        }

        if (obj == m_rollerFan && event == LV_EVENT_VALUE_CHANGED)
        {
            auto selected = lv_roller_get_selected(m_rollerFan);
            if (selected == 0)
            {
                Log::debug("Setting to EU");
                m_tvBGone.setRegion(TVBGone::EU);
            }
            else if (selected == 1)
            {
                Log::debug("Setting to NA");
                m_tvBGone.setRegion(TVBGone::NA);
            }
            else
            {
                Log::errorf("Unexpected region code: %d", selected);
            }
        }
    }

    void TVBGoneApp::updateUI()
    {
    }

    void TVBGoneApp::setupApp()
    {

        // Turn on the backlight
        getWatch()->openBL();
        m_btnTempUp = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnTempUp, _internalEventHandler);
        lv_obj_align(m_btnTempUp, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_btn_set_fit2(m_btnTempUp, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t *labelTempUp = lv_label_create(m_btnTempUp, NULL);
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

    const char *TVBGoneApp::loopApp()
    {
        lv_task_handler();
        delay(5);
        return nullptr;
    }
}