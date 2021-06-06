#pragma once
#include "App.h"
#include <LilyGoWatch.h>

namespace app
{
    class AbstractApp : public IApp
    {
    protected:
        AbstractApp()
            : m_watch(TTGOClass::getWatch())
        {
        }

        virtual TTGOClass *getWatch()
        {
            return m_watch;
        }

    private:
        TTGOClass *m_watch;
    };
}