#include <Log.h>
#include <WiFi.h>
#include "WifiSettingsApp.h"
#include "MainMenuApp.h"

namespace app
{
    const char WifiSettingsApp::ID[] = "app.wifisettings";

    // TODO: proper destructor
    WifiSettingsApp *WifiSettingsApp::s_instance = nullptr;

    WifiSettingsApp::WifiSettingsApp()
        : m_btnScan(nullptr),
          m_btnConnect(nullptr),
          m_rollerWifiSSID(nullptr),
          m_textPassword(nullptr),
          m_keyboard(nullptr)

    {
        // HACK: for the event listeners as they can only be static functions
        s_instance = this;
    }

    WifiSettingsApp::~WifiSettingsApp()
    {
        //lv_obj_del(m_btnmatrixMenu);
        s_instance = nullptr;
    }

    WifiSettingsApp *WifiSettingsApp::instance()
    {
        return s_instance;
    }

    void WifiSettingsApp::buttonEventHandler(lv_obj_t *obj, lv_event_t event)
    {
        Log::debugf("event %d", event);
        if (obj == m_btnScan)
        {
            if (event == LV_EVENT_CLICKED)
            {
                scanWifi();
            }
        }

        if (obj == m_keyboard)
        {
            Log::debug("keyboard event");
            lv_keyboard_def_event_cb(m_keyboard, event);
            if (event == LV_EVENT_APPLY)
            {
                Log::debug("apply");
            }

            if (event == LV_EVENT_CANCEL)
            {
                lv_keyboard_set_textarea(m_keyboard, NULL);
                lv_obj_del(m_keyboard);
                m_keyboard = nullptr;
            }
        }

        if (obj == m_textPassword)
        {
            Log::debug("pass event");
            if (event == LV_EVENT_CLICKED)
            {
                Log::debug("set keyboard to text area");
                /* Focus on the clicked text area */
                if (m_keyboard == nullptr)
                {
                    Log::debug("Create keyboard");
                    createKeyboard();
                }

                // asign to text area
                lv_keyboard_set_textarea(m_keyboard, m_textPassword);
            }
            else if (event == LV_EVENT_INSERT)
            {
                Log::debug("event_insert");
                const char *str = (const char *)lv_event_get_data();
                if (str[0] == '\n')
                {
                    printf("Ready\n");
                }
            }
        }

        if (obj == m_btnConnect)
        {
            if (event == LV_EVENT_CLICKED)
            {
                const char *password = lv_textarea_get_text(m_textPassword);
                char ssid[128];
                lv_roller_get_selected_str(m_rollerWifiSSID, &ssid[0], sizeof(ssid));
                Log::infof("SSID: %s", ssid);
                Log::infof("Pass: %s", password);

                // TODO: refactor this part into wifi manager class
                // TODO: implement timeout
                WiFi.disconnect();
                WiFi.begin(ssid, password);
                Log::info("Connecting");
                while (WiFi.status() != WL_CONNECTED)
                {
                    delay(500);
                    Log::infof("Wifi Status: %d", WiFi.status());
                }
                Log::infof("Connected, IP address: %s", WiFi.localIP().toString().c_str());
            }
        }
    }

    void WifiSettingsApp::createKeyboard()
    {
        m_keyboard = lv_keyboard_create(lv_scr_act(), NULL);
        lv_obj_set_size(m_keyboard, LV_HOR_RES, LV_VER_RES / 2);
        lv_obj_set_event_cb(m_keyboard, _internalEventHandler);

        lv_keyboard_set_cursor_manage(m_keyboard, true); // Automatically show/hide cursors on text areas
    }

    void WifiSettingsApp::scanWifi()
    {
        int n = WiFi.scanNetworks();
        Serial.println("scan done");
        String wifis = "";
        if (n == 0)
        {
            Serial.println("no networks found");
        }
        else
        {
            Serial.print(n);
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i)
            {
                // Print SSID and RSSI for each network found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                wifis += WiFi.SSID(i);
                if (i < n - 1)
                {
                    wifis += "\n";
                }
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.print(")");
                Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
                delay(10);
            }
        }

        lv_roller_set_options(m_rollerWifiSSID, wifis.c_str(), LV_ROLLER_MODE_NORMAL);
        Serial.println("");
    }

    void WifiSettingsApp::setupApp()
    {
        // force full screen refresh
        lv_scr_load(lv_scr_act());

        m_rollerWifiSSID = lv_roller_create(lv_scr_act(), NULL);
        lv_roller_set_options(m_rollerWifiSSID,
                              "",
                              LV_ROLLER_MODE_NORMAL);
        lv_roller_set_fix_width(m_rollerWifiSSID, 200);
        lv_roller_set_visible_row_count(m_rollerWifiSSID, 4);
        lv_obj_align(m_rollerWifiSSID, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
        lv_obj_set_event_cb(m_rollerWifiSSID, _internalEventHandler);

        m_btnScan = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnScan, _internalEventHandler);
        //lv_btn_set_fit2(m_btnScan, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t *labelScan = lv_label_create(m_btnScan, NULL);
        lv_label_set_text(labelScan, "Scan");
        lv_obj_set_width(m_btnScan, 80);

        m_btnConnect = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnConnect, _internalEventHandler);
        //lv_btn_set_fit2(m_btnConnect, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t *labelConnect = lv_label_create(m_btnConnect, NULL);
        lv_label_set_text(labelConnect, "Connect");
        lv_obj_set_width(m_btnConnect, 80);

        //lv_obj_set_auto_realign(m_btnScan, true);
        lv_obj_align(m_btnScan, m_rollerWifiSSID, LV_ALIGN_OUT_BOTTOM_MID, -45, 0);

        //lv_obj_set_auto_realign(m_btnConnect, true);
        lv_obj_align(m_btnConnect, m_rollerWifiSSID, LV_ALIGN_OUT_BOTTOM_MID, 45, 0);

        lv_obj_t *labelPassword = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(labelPassword, "Password: ");
        lv_obj_align(labelPassword, NULL, LV_ALIGN_IN_TOP_LEFT, 25, 10);

        m_textPassword = lv_textarea_create(lv_scr_act(), NULL);
        lv_textarea_set_text(m_textPassword, "");
        lv_textarea_set_pwd_mode(m_textPassword, true);
        lv_textarea_set_one_line(m_textPassword, true);
        lv_textarea_set_cursor_hidden(m_textPassword, true);
        lv_obj_set_width(m_textPassword, LV_HOR_RES / 2 - 20);
        lv_obj_set_pos(m_textPassword, 5, 20);
        lv_obj_set_event_cb(m_textPassword, _internalEventHandler);

        lv_obj_align(m_textPassword, labelPassword, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    }

    const char *WifiSettingsApp::loopApp()
    {
        lv_task_handler();
        delay(5);
        return nullptr;
    }
}