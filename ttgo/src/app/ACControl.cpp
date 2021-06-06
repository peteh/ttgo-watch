#include "ACControl.h"

namespace app
{
    void ACControl::setup()
    {
        m_watch = TTGOClass::getWatch();
        m_watch->begin();
        // Turn on the backlight
        m_watch->openBL();

        //Receive objects for easy writing
        m_tft = m_watch->tft;

        uint8_t pin TWATCH_2020_IR_PIN;
        m_pin = 13;
        m_on = false;
        pinMode(m_pin, OUTPUT);
        digitalWrite(m_pin, m_on);
    }

    void ACControl::loop()
    {
        Serial.println("Sending IR command to A/C ...");
        //m_ac->send();
        m_on = !m_on;
        digitalWrite(m_pin, m_on);
        delay(2000);
    }
}