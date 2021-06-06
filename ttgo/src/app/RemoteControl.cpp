#include "RemoteControl.h"
#include <Log.h>

namespace app
{
    void RemoteControl::setup()
    {
        m_watch = TTGOClass::getWatch();

        // Initialize the hardware, the BMA423 sensor has been initialized internally
        m_watch->begin();

        // Turn on the backlight
        m_watch->openBL();

        //Receive objects for easy writing
        m_tft = m_watch->tft;
        m_bmaSensor = m_watch->bma;

        // Accel parameter structure
        Acfg cfg;
        /*!
        Output data rate in Hz, Optional parameters:
            - BMA4_OUTPUT_DATA_RATE_0_78HZ
            - BMA4_OUTPUT_DATA_RATE_1_56HZ
            - BMA4_OUTPUT_DATA_RATE_3_12HZ
            - BMA4_OUTPUT_DATA_RATE_6_25HZ
            - BMA4_OUTPUT_DATA_RATE_12_5HZ
            - BMA4_OUTPUT_DATA_RATE_25HZ
            - BMA4_OUTPUT_DATA_RATE_50HZ
            - BMA4_OUTPUT_DATA_RATE_100HZ
            - BMA4_OUTPUT_DATA_RATE_200HZ
            - BMA4_OUTPUT_DATA_RATE_400HZ
            - BMA4_OUTPUT_DATA_RATE_800HZ
            - BMA4_OUTPUT_DATA_RATE_1600HZ
    */
        cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
        /*!
        G-range, Optional parameters:
            - BMA4_ACCEL_RANGE_2G
            - BMA4_ACCEL_RANGE_4G
            - BMA4_ACCEL_RANGE_8G
            - BMA4_ACCEL_RANGE_16G
    */
        cfg.range = BMA4_ACCEL_RANGE_2G;
        /*!
        Bandwidth parameter, determines filter configuration, Optional parameters:
            - BMA4_ACCEL_OSR4_AVG1
            - BMA4_ACCEL_OSR2_AVG2
            - BMA4_ACCEL_NORMAL_AVG4
            - BMA4_ACCEL_CIC_AVG8
            - BMA4_ACCEL_RES_AVG16
            - BMA4_ACCEL_RES_AVG32
            - BMA4_ACCEL_RES_AVG64
            - BMA4_ACCEL_RES_AVG128
    */
        cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

        /*! Filter performance mode , Optional parameters:
        - BMA4_CIC_AVG_MODE
        - BMA4_CONTINUOUS_MODE
    */
        cfg.perf_mode = BMA4_CONTINUOUS_MODE;

        // Configure the BMA423 accelerometer
        m_bmaSensor->accelConfig(cfg);

        // Enable BMA423 accelerometer
        // Warning : Need to use feature, you must first enable the accelerometer
        // Warning : Need to use feature, you must first enable the accelerometer
        // Warning : Need to use feature, you must first enable the accelerometer
        m_bmaSensor->enableAccel();
        m_remote.init();

        m_left = 0.;
        m_right = 0;
    }

    void RemoteControl::loop()
    {
        // Obtain the BMA423 direction,
        // so that the screen orientation is consistent with the sensor
        Accel acc;
        m_bmaSensor->getAccel(acc);
        int16_t touchX;
        int16_t touchY;
        bool isTouched = m_watch->getTouch(touchX, touchY);
        Log::infof("Acc: %d, %d, %d", acc.x, acc.y, acc.z);

        uint8_t rotation = m_bmaSensor->direction();
        if (m_prevRotation != rotation)
        {
            m_prevRotation = rotation;
            Serial.println(rotation);
            switch (rotation)
            {
            case DIRECTION_DISP_DOWN:
                //No use
                break;
            case DIRECTION_DISP_UP:
                // stop
                m_tft->setRotation(4);
                m_left = 0.;
                m_right = 0;
                break;
            case DIRECTION_BOTTOM_EDGE:
                // right
                m_left = 1.;
                m_right = -1.;
                m_tft->setRotation(1);
                break;
            case DIRECTION_TOP_EDGE:
                // left
                m_left = -1.;
                m_right = 1.;
                m_tft->setRotation(0);
                break;
            case DIRECTION_RIGHT_EDGE:
                // backwards
                m_left = -1.;
                m_right = -1.;
                m_tft->setRotation(3);
                break;
            case DIRECTION_LEFT_EDGE:
                // forwarnd
                m_left = 1.;
                m_right = 1.;
                m_tft->setRotation(2);
                break;
            default:
                break;
            }

            m_tft->fillScreen(TFT_BLACK);
            m_tft->drawCentreString("T-Watch", 120, 120, 4);
        }
        m_remote.send(m_left, m_right, isTouched);
        m_remote.loop();
        delay(50);
    }
}