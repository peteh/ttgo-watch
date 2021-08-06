#include "RemoteControlApp.h"
#include <Log.h>

namespace app
{
    const char RemoteControlApp::ID[] = "app.remotecontrol";

    RemoteControlApp::RemoteControlApp()
        : m_left(0.0),
          m_right(0.0),
          m_timestampLastSend(millis()),
          m_imuPoint(nullptr)
    {
    }

    RemoteControlApp::~RemoteControlApp()
    {
        lv_obj_del(m_imuPoint);
    }

    void RemoteControlApp::setupApp()
    {
        Log::debug("Setup app");
        // Turn on the backlight
        getWatch()->openBL();

        //Receive objects for easy writing
        m_tft = getWatch()->tft;
        m_bmaSensor = getWatch()->bma;

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
        m_bmaSensor->enableAccel();
        m_remote.init();

        m_left = 0.;
        m_right = 0;
        Log::debug("end Setup app");
        m_imuPoint  = lv_led_create(lv_scr_act(), NULL);
        lv_obj_align(m_imuPoint, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_led_on(m_imuPoint);
    }

    const char *RemoteControlApp::loopApp()
    {
        // TODO: handle not connected wifi
        lv_task_handler();

        // Obtain the BMA423 direction,
        // so that the screen orientation is consistent with the sensor
        Accel acc;
        m_bmaSensor->getAccel(acc);
        
        //Log::infof("Acc: %d, %d, %d", acc.x, acc.y, acc.z);
        float xNormalized = - acc.x / 1024.;
        float yNormalized = acc.y / 1024. ;

        Log::infof("Acc: %.2f, %.2f", xNormalized, yNormalized);

        lv_obj_align(m_imuPoint, NULL, LV_ALIGN_CENTER, (int) (-yNormalized * LV_HOR_RES / 2. ), (int) (-xNormalized * LV_VER_RES / 2.));
        uint8_t rotation = m_bmaSensor->direction();
        if (m_prevRotation != rotation)
        {
            m_prevRotation = rotation;
            switch (rotation)
            {
            case DIRECTION_DISP_DOWN:
                //No use
                m_left = 0.;
                m_right = 0;
                break;
            case DIRECTION_DISP_UP:
                // stop
                m_left = 0.;
                m_right = 0;
                break;
            case DIRECTION_BOTTOM_EDGE:
                // left
                m_left = -1.;
                m_right = 1.;
                break;
            case DIRECTION_TOP_EDGE:
                // right
                m_left = 1.;
                m_right = -1.;
                break;
            case DIRECTION_RIGHT_EDGE:
                // forward
                m_left = 1.;
                m_right = 1.;
                break;
            case DIRECTION_LEFT_EDGE:
                // backwards
                m_left = -1.;
                m_right = -1.;
                break;
            default:
                break;
            }
            // TODO: update the visualization
            //m_tft->drawCentreString("T-Watch", 120, 120, 4);
        }
        if (millis() - m_timestampLastSend >= 20)
        {
            bool isTouched = getWatch()->touched();
            m_timestampLastSend = millis();
            //Log::debugf("Left: %f, Right: %f", m_left, m_right);
            m_remote.send(m_left, m_right, isTouched);
        }
        m_remote.loop();
        delay(5);
        return nullptr;
    }
}