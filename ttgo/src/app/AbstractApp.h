#pragma once
#include "../config.h"
#include <LilyGoWatch.h>

#include "App.h"
#include "WatchButton.h"
#include "../WifiManager.h"

namespace app
{
    class AbstractApp : public IApp
    {
    protected:
        AbstractApp()
            : m_watch(TTGOClass::getWatch()),
            m_watchButtonPresses(0),
            m_watchButton()
        {
        }

        void vibrateHaptic()
        {
            getWatch()->motor->onec(30);
        }

        virtual TTGOClass *getWatch()
        {
            return m_watch;
        }

        virtual uint watchButtonWasPressed()
        {
            return m_watchButtonPresses;
        }

        virtual twatch::WifiManager& getWifiManager()
        {
            return twatch::WifiManager::instance();
        }

        virtual void setup();
        
        virtual const char* loop();

        virtual void tearDown();

        // may be implemented by app
        virtual void setupApp()
        {
            
        }

        // must be implemented by app
        virtual const char* loopApp() = 0;

        // may be implemented by app
        virtual void tearDownApp()
        {

        }

        virtual ~AbstractApp()
        {
            
        }

    private:
        static volatile bool s_powerInterrupt;
        TTGOClass *m_watch;


        uint m_watchButtonPresses;
        WatchButton m_watchButton;
    };
}