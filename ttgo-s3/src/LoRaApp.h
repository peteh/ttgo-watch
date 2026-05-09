#include <lvgl.h>
#include "App.h"
class LoRaApp : public App
{
public:
    enum class EventType
    {
        STATUS = 0, 
        BUTTON = 1
    };

    LoRaApp() = default;
    void setup() override;
    void loop() override;
    bool networkJoin();
    void sendUplink(EventType eventType);
    void buttonEventCallback(lv_event_t *e);
    ~LoRaApp() = default;

private:
    lv_obj_t *m_btn = nullptr;
    bool m_joined = false;
    uint32_t m_lastTx = 0;
};
