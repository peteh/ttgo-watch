#pragma once
#include "AbstractApp.h"
namespace app
{
    class ClockDigitalApp : public AbstractApp
    {
    public:
        const static char ID[];
        ClockDigitalApp();

        virtual void setupApp() override;
        virtual const char *loopApp() override;

    private:
        long m_targetTime;
        boolean m_initial;
        byte omm = 99;
    
    byte xcolon = 0;
    unsigned int colour = 0;
    };
}