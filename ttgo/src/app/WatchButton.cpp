#include "WatchButton.h"
#include <Log.h>

namespace app
{
    const long WatchButton::PRESS_DELAY = 300;
const long WatchButton::MAX_CLICKS = 2;
    WatchButton::WatchButton()
        : m_presses(0),
          m_pressTS(0)
    {
    }

    uint WatchButton::evaluate(bool pressed)
    {
        if (pressed)
        {
            if (millis() - m_pressTS < PRESS_DELAY)
            {
                m_presses++;
            }
            else
            {
                m_presses = 1;
            }
            m_pressTS = millis();
            Log::debug("Counting up");
            return 0;
        }

        if (millis() - m_pressTS > PRESS_DELAY || m_presses >= MAX_CLICKS)
        {
            uint presses = m_presses;
            m_presses = 0;
            return presses;
        }

        return 0;
    }
}