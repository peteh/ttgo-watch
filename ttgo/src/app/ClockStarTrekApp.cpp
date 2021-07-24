#include "ClockStarTrekApp.h"
#include "MainMenuApp.h"

#include <time.h>

namespace app
{
    const char ClockStarTrekApp::ID[] = "app.startrekclock";

    ClockStarTrekApp::ClockStarTrekApp()
    :m_fullUpdate(true)
    {
        setWatchButtonApp(MainMenuApp::ID);
    }

    void ClockStarTrekApp::setupApp()
    {
        drawBase();
    }

    void ClockStarTrekApp::drawBase()
    {
        int32_t topWallWidth = 20;
        //Draw the back graphics - Top of display
        getWatch()->tft->fillScreen(TFT_BLACK);
        getWatch()->tft->fillRoundRect(0, 0, 239, 120, 40, TFT_PURPLE);
        getWatch()->tft->fillRoundRect(topWallWidth, 20, 196, 80, 20, TFT_BLACK);
        getWatch()->tft->fillRect(80, 20, 159, 80, TFT_BLACK);
        getWatch()->tft->fillRect(170, 0, 45, 20, TFT_BLACK);
        getWatch()->tft->fillRect(110, 0, 4, 20, TFT_BLACK);
        getWatch()->tft->fillRect(0, 45, 50, 7, TFT_BLACK);
        getWatch()->tft->fillRect(0, 70, 50, 7, TFT_BLACK);
        getWatch()->tft->fillRect(215, 0, 24, 20, TFT_DARKCYAN);

        //Draw the back graphics - Bottom of display
        getWatch()->tft->fillRoundRect(0, 130, 239, 109, 40, TFT_MAROON);
        getWatch()->tft->fillRoundRect(40, 150, 199, 88, 20, TFT_BLACK);
        getWatch()->tft->fillRect(0, 179, 50, 10, TFT_BLACK);
        getWatch()->tft->fillRect(100, 160, 40, 10, TFT_YELLOW);
        getWatch()->tft->fillRect(140, 160, 40, 10, TFT_DARKGREEN);
        getWatch()->tft->fillRect(180, 160, 40, 10, TFT_RED);
        getWatch()->tft->setTextColor(TFT_WHITE, TFT_BLACK);
        getWatch()->tft->drawString("Temp", 66, 158, 2);
        getWatch()->tft->fillRoundRect(119, 210, 120, 29, 15, TFT_DARKCYAN);
        m_fullUpdate = false;
    }

    const char *ClockStarTrekApp::loopApp()
    {
        if(getWatch()->touched())
        {
            return MainMenuApp::ID;
        }
        getWatch()->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

        // Get the current data
        RTC_Date tnow = getWatch()->rtc->getDateTime();

        uint8_t hh = tnow.hour;
        uint8_t mm = tnow.minute;
        uint8_t ss = tnow.second;
        uint8_t dday = tnow.day;
        uint8_t mmonth = tnow.month;
        uint16_t yyear = tnow.year;

        getWatch()->tft->setTextSize(1);
        if (m_fullUpdate)
        {
            drawBase();
        }
        // Display Temp Marker - you may need to adjust the x value based on your normal ADC results
        float temp = getWatch()->power->getTemp();
        getWatch()->tft->fillRoundRect(int(temp) - 20, 170, 10, 20, 5, TFT_WHITE);

        // Display Time
        // Font 7 is a 7-seg display but only contains
        // characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .

        getWatch()->tft->setTextColor(0xFBE0, TFT_BLACK);
        uint8_t font = 7;
        int xpos = 22;
        if (hh < 10)
        {
            xpos += getWatch()->tft->drawChar('0', xpos, 35, font);
        }
        xpos += getWatch()->tft->drawNumber(hh, xpos, 35, font);
        xpos += 3;
        xpos += getWatch()->tft->drawChar(':', xpos, 35, font);

        if (mm < 10)
        {
            xpos += getWatch()->tft->drawChar('0', xpos, 35, font);
        }
        xpos += getWatch()->tft->drawNumber(mm, xpos, 35, font);
        xpos += 3;
        xpos += getWatch()->tft->drawChar(':', xpos, 35, font);

        if (ss < 10)
        {
            xpos += getWatch()->tft->drawChar('0', xpos, 35, font);
        }
        getWatch()->tft->drawNumber(ss, xpos, 35, font);

        // Display Battery Level
        getWatch()->tft->setTextSize(1);
        getWatch()->tft->setTextColor(TFT_YELLOW);
        getWatch()->tft->drawString("Power", 124, 2, 2);
        getWatch()->tft->setTextColor(TFT_GREEN, TFT_BLACK);
        int per = getWatch()->power->getBattPercentage();
        per = getWatch()->power->getBattPercentage();
        getWatch()->tft->drawString(String(per) + "%", 179, 2, 2);
        getWatch()->tft->setTextColor(TFT_GREENYELLOW);
        getWatch()->tft->drawString(String(dday), 50, 188, 6);

        // Turn off the battery adc
        getWatch()->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, false);

        // Draw Month
        String mStr;
        switch (mmonth)
        {
        case 1:
            mStr = "Jan";
            break;
        case 2:
            mStr = "Feb";
            break;
        case 3:
            mStr = "Mar";
            break;
        case 4:
            mStr = "Apr";
            break;
        case 5:
            mStr = "May";
            break;
        case 6:
            mStr = "Jun";
            break;
        case 7:
            mStr = "Jul";
            break;
        case 8:
            mStr = "Aug";
            break;
        case 9:
            mStr = "Sep";
            break;
        case 10:
            mStr = "Oct";
            break;
        case 11:
            mStr = "Nov";
            break;
        case 12:
            mStr = "Dec";
            break;
        }
        getWatch()->tft->setTextColor(TFT_WHITE);
        getWatch()->tft->drawString(mStr, 9, 194, 2);

        // Build a bargraph every 10 seconds
        int secmod = ss % 10;
        if (secmod)
        { // Show growing bar every 10 seconds
            getWatch()->tft->fillRect(126 + secmod * 10, 215, 6, 15, TFT_ORANGE);
        }
        else
        {
            getWatch()->tft->fillRoundRect(119, 210, 120, 29, 15, TFT_DARKCYAN);
        }
        return nullptr;
    }
}