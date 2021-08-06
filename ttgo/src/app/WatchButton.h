#pragma once
#include <Arduino.h>

namespace app
{
    class WatchButton
    {
    public:
        WatchButton();
        uint evaluate(bool pressed);

    private:
        uint m_presses;
        long m_pressTS;

        static const long PRESS_DELAY;
        static const long MAX_CLICKS;
    };
}