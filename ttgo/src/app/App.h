#pragma once
#define LILYGO_WATCH_2020_V3
// => Function select
#define LILYGO_WATCH_LVGL //To use LVGL, you need to enable the macro LVGL

namespace app
{
    class IApp
    {
        public: 
        virtual void setup() = 0;
        virtual void loop() = 0;
    };
}
