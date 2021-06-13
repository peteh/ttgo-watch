#pragma once
#include "AbstractApp.h"
namespace app
{
    class ClockStarTrekApp : public AbstractApp
    {
    public:
        const static char ID[];
        ClockStarTrekApp();

        virtual void setupApp() override;
        virtual const char *loopApp() override;

    private:
    void drawBase();
    bool m_fullUpdate;
    };
}