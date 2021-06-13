#include "ACControlApp.h"
#include <Log.h>
#include <ArduinoJson.h>
#include <FS.h>

namespace app
{
    const char ACControlApp::ID[] = "app.accontroll";

    // TODO: proper destructor
    ACControlApp *ACControlApp::s_instance = nullptr;

    ACControlApp::ACControlApp()
        : AbstractApp(),
          m_configChanged(false),
          m_timestampLastSave(millis())
    {
        // HACK: for the event listeners as they can only be static functions
        s_instance = this;
    }

    ACControlApp::~ACControlApp()
    {
        Log::debug("destructor 1");
        s_instance = nullptr;
        delete m_ac;

        // TODO: rather use parents to delete children in lvgl
        // Buttons:

        lv_obj_del(m_btnTempUp);
        lv_obj_del(m_btnTempDown);


        lv_obj_del(m_switchPower);
        lv_obj_del(m_labelPower);

        lv_obj_del(m_switchTurbo);
        lv_obj_del(m_labelTurbo);

        lv_obj_del(m_switchSwing);
        lv_obj_del(m_labelSwing);

        lv_obj_del(m_dropdownMode);

        lv_obj_del(m_labelTemp);

        lv_obj_del(m_rollerFan);
    }

    ACControlApp *ACControlApp::instance()
    {
        return s_instance;
    }

    void ACControlApp::loadFromConfig()
    {
        std::unique_lock<std::mutex> lock(m_mutexFile);

        fs::File configFile = SPIFFS.open("/app.ACControl.cfg", "r");
        if (!configFile)
        {
            Log::info("Failed to open config file");
            return;
        }

        size_t size = configFile.size();
        Log::infof("Config size: %d", size);
        if (size > 256)
        {
            Log::error("Config file size is too large");
            return;
        }

        // Allocate the memory pool on the stack.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<256> doc;
        // Deserialize the JSON document
        DeserializationError jsonError = deserializeJson(doc, configFile);
        configFile.close();
        if (jsonError)
        {
            Log::error("Failed to read file, using default configuration");
            return;
        }

        bool on = doc["on"] | true;
        on ? m_ac->on() : m_ac->off();
        m_ac->setMode(doc["mode"] | kGreeCool);
        m_ac->setTemp(doc["temp"] | 25);
        m_ac->setFan(doc["fan"] | 0); // auto
        m_ac->setTurbo(doc["turbo"] | false);
        m_ac->setSwingVertical(doc["swingAuto"] | false, doc["swingPos"] | kGreeSwingMiddle);
        Log::debug("Finished loading config");
    }

    void ACControlApp::saveToConfig()
    {
        if (!m_configChanged)
        {
            return;
        }

        std::unique_lock<std::mutex> lock(m_mutexFile);
        m_configChanged = false;
        m_timestampLastSave = millis();

        fs::File configFile = SPIFFS.open("/app.ACControl.cfg", "w");
        if (!configFile)
        {
            Log::info("Failed to open config file");
            return;
        }

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<256> doc;

        // Set the values in the document
        doc["on"] = m_ac->getPower();

        doc["temp"] = m_ac->getTemp();
        doc["fan"] = m_ac->getFan();
        doc["turbo"] = m_ac->getTurbo();

        doc["swingAuto"] = m_ac->getSwingVerticalAuto();
        doc["swingPos"] = m_ac->getSwingVerticalPosition();

        doc["mode"] = m_ac->getMode();

        // Serialize JSON to file
        if (serializeJson(doc, configFile) == 0)
        {
            Log::error("Failed to write to file");
        }

        // Close the file
        configFile.close();
        Log::debug("Saved Config file");
    }

    void ACControlApp::buttonEventHandler(lv_obj_t *obj, lv_event_t event)
    {
        if (event == LV_EVENT_CLICKED)
        {
            if (obj == m_btnTempUp)
            {
                m_ac->setTemp(m_ac->getTemp() + 1);
                m_ac->on();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }
            if (obj == m_btnTempDown)
            {
                m_ac->setTemp(m_ac->getTemp() - 1);
                m_ac->on();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }
        }
        else if (event == LV_EVENT_VALUE_CHANGED)
        {
            if (obj == m_switchPower)
            {
                Log::debugf("On: %d", obj->state);
                (lv_switch_get_state(m_switchPower)) ? m_ac->on() : m_ac->off();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }

            if (obj == m_switchTurbo)
            {
                m_ac->setTurbo(lv_switch_get_state(m_switchTurbo));
                m_ac->on();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }

            if (obj == m_switchSwing)
            {
                if (lv_switch_get_state(m_switchSwing))
                {
                    m_ac->setSwingVertical(true, kGreeSwingAuto);
                }
                else
                {
                    m_ac->setSwingVertical(false, kGreeSwingMiddle);
                }
                m_ac->on();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }

            if (obj == m_dropdownMode)
            {
                uint8_t mode = getModeFromSelection(lv_dropdown_get_selected(m_dropdownMode));
                Log::debugf("Mode: %d", mode);
                m_ac->setMode(mode);

                m_ac->on();
                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }

            if (obj == m_rollerFan)
            {
                // index maps to actual value, so no conversion needed
                uint8_t value = lv_roller_get_selected(m_rollerFan);
                m_ac->setFan(value);
                Log::debugf("Set Fan: %d", value);

                updateUI();
                vibrateHaptic();

                m_configChanged = true;
                m_ac->send();
            }
        }
    }

    uint8_t ACControlApp::getModeFromSelection(uint index)
    {
        // 0          1          2          3          4
        // Cool       Heat       Auto       Dry        Fan");
        // kGreeCool  kGreeHeat  kGreeAuto  kGreeDry   kGreeFan
        switch (index)
        {
        case 0:
            return kGreeCool;
        case 1:
            return kGreeHeat;
        case 2:
            return kGreeAuto;
        case 3:
            return kGreeDry;
        case 4:
            return kGreeFan;
        default:
            Log::errorf("Unknown selection for mode: %d", index);
            return kGreeAuto;
        }
    }

    uint8_t ACControlApp::getSelectionFromMode(uint mode)
    {
        // 0          1          2          3          4
        // Cool       Heat       Auto       Dry        Fan");
        // kGreeCool  kGreeHeat  kGreeAuto  kGreeDry   kGreeFan
        switch (mode)
        {
        case kGreeCool:
            return 0;
        case kGreeHeat:
            return 1;
        case kGreeAuto:
            return 2;
        case kGreeDry:
            return 3;
        case kGreeFan:
            return 4;
        default:
            Log::errorf("Unknown selection for mode: %d", index);
            return 2;
        }
    }

    void ACControlApp::updateUI()
    {
        // Power switch
        if (m_ac->getPower() != lv_switch_get_state(m_switchPower))
        {
            (m_ac->getPower()) ? lv_switch_on(m_switchPower, LV_ANIM_ON) : lv_switch_off(m_switchPower, LV_ANIM_ON);
        }

        // turbo switch
        if (m_ac->getTurbo() != lv_switch_get_state(m_switchTurbo))
        {
            (m_ac->getTurbo()) ? lv_switch_on(m_switchTurbo, LV_ANIM_ON) : lv_switch_off(m_switchTurbo, LV_ANIM_ON);
        }

        // swing switch
        // switch enabled: auto on, move auto
        if (!(lv_switch_get_state(m_switchSwing) && m_ac->getSwingVerticalAuto() && m_ac->getSwingVerticalPosition() == kGreeSwingAuto) &&
            !(!lv_switch_get_state(m_switchSwing) && !m_ac->getSwingVerticalAuto() && m_ac->getSwingVerticalPosition() == kGreeSwingMiddle))
        {
            if (m_ac->getSwingVerticalAuto() && m_ac->getSwingVerticalPosition() == kGreeSwingAuto)
            {
                lv_switch_on(m_switchSwing, LV_ANIM_ON);
            }
            else
            {
                lv_switch_off(m_switchSwing, LV_ANIM_ON);
            }
        }

        if (m_ac->getFan() != lv_roller_get_selected(m_rollerFan))
        {
            lv_roller_set_selected(m_rollerFan, m_ac->getFan(), LV_ANIM_ON);
            Log::debugf("Fan value: %d", m_ac->getFan());
        }

        lv_label_set_text_fmt(m_labelTemp, "%d°C", m_ac->getTemp());

        if (getModeFromSelection(lv_dropdown_get_selected(m_dropdownMode)) != m_ac->getMode())
        {
            lv_dropdown_set_selected(m_dropdownMode, getSelectionFromMode(m_ac->getMode()));
        }
    }

    void ACControlApp::setupApp()
    {
        // TODO: update ui based on internal state
        m_watch = TTGOClass::getWatch();

        // Turn on the backlight
        m_watch->openBL();

        m_ac = new IRGreeAC(TWATCH_2020_IR_PIN);
        m_ac->begin();
        m_ac->on();
        m_ac->setFan(0); // 0 - auto

        // kGreeAuto, kGreeDry, kGreeCool, kGreeFan, kGreeHeat
        m_ac->setMode(kGreeCool);
        m_ac->setTemp(25); // 16-30C
        m_ac->setSwingVertical(false, kGreeSwingMiddle);
        m_ac->setXFan(true); // lets ac run to dry for a few more min after power off
        m_ac->setLight(true);
        //m_ac->setSleep(false);

        m_ac->setTurbo(false);

        m_btnTempUp = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnTempUp, _internalEventHandler);
        lv_obj_align(m_btnTempUp, NULL, LV_ALIGN_CENTER, 35, 0);
        lv_btn_set_fit2(m_btnTempUp, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t* labelTempUp = lv_label_create(m_btnTempUp, NULL);
        lv_label_set_text(labelTempUp, "+");

        m_btnTempDown = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_btnTempDown, _internalEventHandler);
        lv_obj_align(m_btnTempDown, NULL, LV_ALIGN_CENTER, -35, 0);
        lv_btn_set_fit2(m_btnTempDown, LV_FIT_TIGHT, LV_FIT_TIGHT);
        lv_obj_t* labelTempDown = lv_label_create(m_btnTempDown, NULL);
        lv_label_set_text(labelTempDown, "-");

        m_switchPower = lv_switch_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_switchPower, _internalEventHandler);
        lv_obj_align(m_switchPower, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 20);
        m_labelPower = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(m_labelPower, "Power");
        lv_obj_set_auto_realign(m_labelPower, true);
        lv_obj_align(m_labelPower, m_switchPower, LV_ALIGN_OUT_TOP_MID, 0, 0);

        /*Create a normal drop down list*/
        m_dropdownMode = lv_dropdown_create(lv_scr_act(), NULL);
        lv_dropdown_set_options(m_dropdownMode,
                                "Cool\n"
                                "Heat\n"
                                "Auto\n"
                                "Dry\n"
                                "Fan");

        lv_obj_align(m_dropdownMode, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 15);
        lv_obj_set_event_cb(m_dropdownMode, _internalEventHandler);

        // temperature label
        m_labelTemp = lv_label_create(lv_scr_act(), NULL);
        lv_obj_align(m_labelTemp, NULL, LV_ALIGN_CENTER, 0, -40);
        lv_label_set_align(m_labelTemp, LV_LABEL_ALIGN_CENTER);
        lv_label_set_text_fmt(m_labelTemp, "%d°C", m_ac->getTemp());

        // Switch for turbo
        m_switchTurbo = lv_switch_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_switchTurbo, _internalEventHandler);
        lv_obj_align(m_switchTurbo, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 20, -35);
        m_labelTurbo = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(m_labelTurbo, "Turbo");
        lv_obj_set_auto_realign(m_labelTurbo, true);
        lv_obj_align(m_labelTurbo, m_switchTurbo, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

        m_switchSwing = lv_switch_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(m_switchSwing, _internalEventHandler);
        lv_obj_align(m_switchSwing, NULL, LV_ALIGN_IN_BOTTOM_MID, -10, -35);
        m_labelSwing = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(m_labelSwing, "Swing");
        lv_obj_set_auto_realign(m_labelSwing, true);
        lv_obj_align(m_labelSwing, m_switchSwing, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

        m_rollerFan = lv_roller_create(lv_scr_act(), NULL);
        lv_roller_set_options(m_rollerFan,
                              "Fan: aut\n"
                              "Fan: 1\n"
                              "Fan: 2\n"
                              "Fan: 3",
                              LV_ROLLER_MODE_NORMAL);

        lv_roller_set_visible_row_count(m_rollerFan, 2);
        lv_obj_align(m_rollerFan, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
        lv_obj_set_event_cb(m_rollerFan, _internalEventHandler);

        // load config if it exists:
        loadFromConfig();

        // force first update
        lv_scr_load(lv_scr_act());
        updateUI();
    }

    const char *ACControlApp::loopApp()
    {
        lv_task_handler();
        delay(5);

        // save every 10 seconds if there were changes
        if (m_configChanged && millis() - m_timestampLastSave > 10000)
        {
            saveToConfig();
        }
        return nullptr;
    }

    void ACControlApp::tearDownApp()
    {
        saveToConfig();
    }
}