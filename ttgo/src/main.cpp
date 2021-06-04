#include <Arduino.h>

// => Hardware select
// #define LILYGO_WATCH_2019_WITH_TOUCH     //To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH       //To use T-Watch2019 Not touchscreen , please uncomment this line
// #define LILYGO_WATCH_2020_V1             //To use T-Watch2020 V1, please uncomment this line
// #define LILYGO_WATCH_2020_V2             //To use T-Watch2020 V2, please uncomment this line
#define LILYGO_WATCH_2020_V3 //To use T-Watch2020 V3, please uncomment this line

// => Function select
#define LILYGO_WATCH_LVGL //To use LVGL, you need to enable the macro LVGL

#include <PubSubClient.h>
#include <LilyGoWatch.h>
#include <Log.h>
#include <SerialLogger.h>
TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
uint8_t prevRotation;

#include "Remote.h"

Remote g_remote;
float g_left = 0.;
float g_right = 0.;

void setup()
{
    Serial.begin(115200);
    Log::init(new SerialLogger());
    // Get TTGOClass instance
    watch = TTGOClass::getWatch();

    // Initialize the hardware, the BMA423 sensor has been initialized internally
    watch->begin();

    // Turn on the backlight
    watch->openBL();

    //Receive objects for easy writing
    tft = watch->tft;
    sensor = watch->bma;

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
    sensor->accelConfig(cfg);

    // Enable BMA423 accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    sensor->enableAccel();
    g_remote.init();
}

void loop()
{
    // Obtain the BMA423 direction,
    // so that the screen orientation is consistent with the sensor
    Accel acc;
    sensor->getAccel(acc);
    int16_t touchX;
    int16_t touchY;
    bool isTouched = watch->getTouch(touchX, touchY);
    Log::infof("Acc: %d, %d, %d", acc.x, acc.y, acc.z);

    uint8_t rotation = sensor->direction();
    if (prevRotation != rotation)
    {
        prevRotation = rotation;
        Serial.println(rotation);
        switch (rotation)
        {
        case DIRECTION_DISP_DOWN:
            //No use
            break;
        case DIRECTION_DISP_UP:
            // stop
            tft->setRotation(4);
            g_left = 0.;
            g_right = 0;
            break;
        case DIRECTION_BOTTOM_EDGE:
            // right
            g_left = 1.;
            g_right = -1.;
            tft->setRotation(1);
            break;
        case DIRECTION_TOP_EDGE:
            // left
            g_left = -1.;
            g_right = 1.;
            tft->setRotation(0);
            break;
        case DIRECTION_RIGHT_EDGE:
            // backwards
            g_left = -1.;
            g_right = -1.;
            tft->setRotation(3);
            break;
        case DIRECTION_LEFT_EDGE:
            // forwarnd
            g_left = 1.;
            g_right = 1.;
            tft->setRotation(2);
            break;
        default:
            break;
        }

        tft->fillScreen(TFT_BLACK);
        tft->drawCentreString("T-Watch", 120, 120, 4);
    }
    g_remote.send(g_left, g_right, isTouched);
    g_remote.loop();
    delay(50);
}