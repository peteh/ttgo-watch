#include "AbstractApp.h"

namespace app
{
    class ButtonTestApp : public AbstractApp
    {
        public: 
        
        virtual void setup() override;
        virtual void loop() override;
    };


}