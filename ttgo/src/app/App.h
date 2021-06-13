#pragma once

namespace app
{
    class IApp
    {
        public: 
        virtual void setup() = 0;
        virtual const char* loop() = 0;
        virtual void tearDown() = 0;
        virtual ~IApp(){};
    };
}
