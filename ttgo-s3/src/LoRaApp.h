#include <lvgl.h>

class LoRaApp
{
public:
    enum class EventType
    {
        STATUS = 0, 
        BUTTON = 1
    };

    LoRaApp() = default;
    void setup();
    void loop();
    void sendUplink(EventType eventType);
    void btn_event_cb(lv_event_t *e);
    ~LoRaApp() = default;

private:
    lv_obj_t *m_btn = nullptr;
    bool m_joined = false;
    uint32_t m_lastTx = 0;
};
