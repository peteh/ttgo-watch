#include "VoiceApp.h"

void VoiceApp::btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;
        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
        Serial.printf("Button :%d\n", cnt);
        m_vp.startRecording();
    }
}

void VoiceApp::setup()
{
    Serial.println("Setting up Voice Pipe...");
    m_vp.init();
    Serial.println("Voice Pipe ready.");

    lv_obj_t *label = lv_label_create(lv_screen_active()); /*Add a label the current screen*/
    lv_label_set_text(label, "Hello World");               /*Set label text*/
    lv_obj_center(label);                                  /*Set center alignment*/

    m_btn = lv_button_create(lv_screen_active()); /*Add a button the current screen*/
    lv_obj_set_size(m_btn, 120, 50);              /*Set its size*/

    lv_obj_add_event_cb(m_btn, [](lv_event_t *e)
                        {
        auto *app = static_cast<VoiceApp *>(lv_event_get_user_data(e));
        app->btn_event_cb(e); }, LV_EVENT_ALL, this);                 /*Assign a callback to the button*/
    lv_obj_align_to(m_btn, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); /*Set the label to it and align it in the center below the label*/

    lv_obj_t *btn_label = lv_label_create(m_btn); /*Add a label to the button*/
    lv_label_set_text(btn_label, "Button");       /*Set the labels text*/
    lv_obj_center(btn_label);
}

void VoiceApp::loop()
{
    m_vp.loop();
    if (m_vp.getState() == VoicePipeline::RECORDING && !m_recording)
    {
        m_recording = true;
        lv_obj_set_style_bg_color(m_btn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (m_vp.getState() != VoicePipeline::RECORDING && m_recording)
    {
        m_recording = false;
        lv_obj_set_style_bg_color(m_btn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}