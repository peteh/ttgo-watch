#include "ClockDigitalApp.h"
#include "MainMenuApp.h"
namespace app
{

    static uint8_t conv2d(const char *p)
    {
        uint8_t v = 0;
        if ('0' <= *p && *p <= '9')
            v = *p - '0';
        return 10 * v + *++p - '0';
    }

    uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

    const char ClockDigitalApp::ID[] = "app.digitalclock";

    ClockDigitalApp::ClockDigitalApp()
        : m_targetTime(0),
          m_initial(true),
          omm(99),
          xcolon(0),
          colour(0)
    {
    }

    void ClockDigitalApp::setupApp()
    {
        getWatch()->tft->setTextFont(1);
        getWatch()->tft->fillScreen(TFT_BLACK);
        getWatch()->tft->setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
        m_targetTime = millis() + 1000;
    }

    const char *ClockDigitalApp::loopApp()
    {
        if (getWatch()->touched())
        {
            return MainMenuApp::ID;
        }
        if (m_targetTime < millis())
        {
            m_targetTime = millis() + 1000;
            ss++; // Advance second
            if (ss == 60)
            {
                ss = 0;
                omm = mm;
                mm++; // Advance minute
                if (mm > 59)
                {
                    mm = 0;
                    hh++; // Advance hour
                    if (hh > 23)
                    {
                        hh = 0;
                    }
                }
            }

            if (ss == 0 || m_initial)
            {
                m_initial = 0;
                getWatch()->tft->setTextColor(TFT_GREEN, TFT_BLACK);
                getWatch()->tft->setCursor(8, 52);
                getWatch()->tft->print(__DATE__); // This uses the standard ADAFruit small font

                getWatch()->tft->setTextColor(TFT_BLUE, TFT_BLACK);
                getWatch()->tft->drawCentreString("It is windy", 120, 48, 2); // Next size up font 2

                //ttgo->tft->setTextColor(0xF81F, TFT_BLACK); // Pink
                //ttgo->tft->drawCentreString("12.34",80,100,6); // Large font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 . : a p m
            }

            // Update digital time
            byte xpos = 6;
            byte ypos = 0;
            if (omm != mm)
            { // Only redraw every minute to minimise flicker
                // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
                getWatch()->tft->setTextColor(0x39C4, TFT_BLACK); // Leave a 7 segment ghost image, comment out next line!
                //ttgo->tft->setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
                // Font 7 is to show a pseudo 7 segment display.
                // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
                getWatch()->tft->drawString("88:88", xpos, ypos, 7); // Overwrite the text to clear it
                getWatch()->tft->setTextColor(0xFBE0, TFT_BLACK);    // Orange
                omm = mm;

                if (hh < 10)
                    xpos += getWatch()->tft->drawChar('0', xpos, ypos, 7);
                xpos += getWatch()->tft->drawNumber(hh, xpos, ypos, 7);
                xcolon = xpos;
                xpos += getWatch()->tft->drawChar(':', xpos, ypos, 7);
                if (mm < 10)
                    xpos += getWatch()->tft->drawChar('0', xpos, ypos, 7);
                getWatch()->tft->drawNumber(mm, xpos, ypos, 7);
            }

            if (ss % 2)
            { // Flash the colon
                getWatch()->tft->setTextColor(0x39C4, TFT_BLACK);
                xpos += getWatch()->tft->drawChar(':', xcolon, ypos, 7);
                getWatch()->tft->setTextColor(0xFBE0, TFT_BLACK);
            }
            else
            {
                getWatch()->tft->drawChar(':', xcolon, ypos, 7);
                colour = random(0xFFFF);
                // Erase the old text with a rectangle, the disadvantage of this method is increased display flicker
                getWatch()->tft->fillRect(0, 64, 160, 20, TFT_BLACK);
                getWatch()->tft->setTextColor(colour);
                getWatch()->tft->drawRightString("Colour", 75, 64, 4); // Right justified string drawing to x position 75
                String scolour = String(colour, HEX);
                scolour.toUpperCase();
                char buffer[20];
                scolour.toCharArray(buffer, 20);
                getWatch()->tft->drawString(buffer, 82, 64, 4);
            }
        }
        return nullptr;
    }
}