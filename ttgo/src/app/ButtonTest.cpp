#include "ButtonTest.h"

namespace app
{
/*
    static void event_handler(lv_obj_t *obj, lv_event_t event)
    {
        if (event == LV_EVENT_CLICKED)
        {
            Serial.printf("Clicked\n");
        }
        else if (event == LV_EVENT_VALUE_CHANGED)
        {
            Serial.printf("Toggled\n");
        }
    }
*/
    void ButtonTestApp::setupApp()
    {
        /*
        getWatch()->begin();
        getWatch()->openBL();
        getWatch()->lvgl_begin();

        lv_obj_t *label;

        lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(btn1, event_handler);
        lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);

        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Button");

        lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(btn2, event_handler);
        lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 40);
        lv_btn_set_checkable(btn2, true);
        lv_btn_toggle(btn2);
        lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

        label = lv_label_create(btn2, NULL);
        lv_label_set_text(label, "Toggled");
        */
    }

    const char* ButtonTestApp::loopApp()
    {
        //lv_task_handler();
        delay(5);
        return nullptr;
    }

}