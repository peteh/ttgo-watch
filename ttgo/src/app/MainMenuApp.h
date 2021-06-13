#pragma once
#include "AbstractApp.h"

namespace app
{
    class MainMenuApp : public AbstractApp
    {
    public:
        const static char ID[];
        static MainMenuApp *instance();

        MainMenuApp();
        virtual void setupApp() override;
        virtual const char *loopApp() override;

        virtual ~MainMenuApp();

    private:
        static MainMenuApp *s_instance;
        static const char *s_btnMap[];
        static const char *s_appIDs[];

        static void _internalEventHandler(lv_obj_t *obj, lv_event_t event)
        {
            instance()->buttonEventHandler(obj, event);
        }

        void buttonEventHandler(lv_obj_t *obj, lv_event_t event);


        int16_t m_menuIndex; // -1 is not selected
        lv_obj_t *m_btnmatrixMenu;
    };

}