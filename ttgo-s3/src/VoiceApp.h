#include "voicepipeline.h"
#include <lvgl.h>

class VoiceApp
{
public:
    VoiceApp() = default;
    void setup();
    void loop();
    void btn_event_cb(lv_event_t *e);
    ~VoiceApp() = default;

private:
    // voice pipeline
    VoicePipeline m_vp;
    lv_obj_t *m_btn = nullptr;
    bool m_recording = false;
};
