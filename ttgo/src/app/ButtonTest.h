#include "AbstractApp.h"

namespace app
{
    class ButtonTestApp : public AbstractApp
    {
        public: 
        
        virtual void setupApp() override;
        virtual const char* loopApp() override;
    };


}