#include <Log.h>
#include "MainMenuApp.h"
#include "ACControlApp.h"
#include "ClockDigitalApp.h"
#include "ClockSettingsApp.h"
#include "ClockStarTrekApp.h"
#include "RemoteControlApp.h"
#include "WifiSettingsApp.h"
#include "TVBGone/TVBGoneApp.h"
namespace app
{
    const char MainMenuApp::ID[] = "app.mainmenu";
    const char *MainMenuApp::s_btnMap[] = {"AC", "Digital", "StarTrek", "\n", 
    "Remote","TVBGone" , "\n"
    "Clock", "WiFi", ""};
    const char *MainMenuApp::s_appIDs[] = {ACControlApp::ID, ClockDigitalApp::ID, ClockStarTrekApp::ID,  
    RemoteControlApp::ID, TVBGoneApp::ID,
    ClockSettingsApp::ID, WifiSettingsApp::ID};

    // TODO: proper destructor
    MainMenuApp *MainMenuApp::s_instance = nullptr;

    MainMenuApp::MainMenuApp()
    : m_menuIndex(-1)
    {
        // HACK: for the event listeners as they can only be static functions
        s_instance = this;
    }

    MainMenuApp::~MainMenuApp()
    {
        lv_obj_del(m_btnmatrixMenu);
        s_instance = nullptr;
    }

    MainMenuApp *MainMenuApp::instance()
    {
        return s_instance;
    }

    void MainMenuApp::buttonEventHandler(lv_obj_t *obj, lv_event_t event)
    {
        if(event == LV_EVENT_VALUE_CHANGED) {
        uint16_t index = lv_btnmatrix_get_active_btn(obj);
        m_menuIndex = index;
        Log::infof("%s was pressed\n", s_appIDs[index]);
    }
    }
    

    void MainMenuApp::setupApp()
    {
        Log::info("Setup Main Menu");
        m_btnmatrixMenu = lv_btnmatrix_create(lv_scr_act(), NULL);
        lv_btnmatrix_set_map(m_btnmatrixMenu, s_btnMap);

        Log::debug("1");
        lv_btnmatrix_set_btn_width(m_btnmatrixMenu, 10, 2); /*Make "Action1" twice as wide as "Action2"*/
        lv_btnmatrix_set_btn_ctrl(m_btnmatrixMenu, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
        lv_btnmatrix_set_btn_ctrl(m_btnmatrixMenu, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
        Log::debug("2");
        lv_obj_align(m_btnmatrixMenu, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_event_cb(m_btnmatrixMenu, _internalEventHandler);
        Log::debug("3");

        // force full screen refresh
        lv_scr_load(lv_scr_act());
    }

    const char *MainMenuApp::loopApp()
    {
        lv_task_handler();
        if(m_menuIndex != -1)
        {
            Log::infof("Switching to app %s", s_appIDs[m_menuIndex]);
            return s_appIDs[m_menuIndex];
        }
        delay(5);
        return nullptr;
    }
}