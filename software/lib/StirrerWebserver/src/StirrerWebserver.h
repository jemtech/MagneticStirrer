#pragma once
#include <ESPAsyncWebServer.h>
#include "StirrerConfig.h"
#include "StirrerControl.h"

class StirrerWebserver {
public:
    StirrerWebserver();
    ~StirrerWebserver();
    void setup(StirrerConfig* config, StirrerControl* control);

private:
    AsyncWebServer* myServer;
    StirrerConfig*  _config  = nullptr;
    StirrerControl* _control = nullptr;
};
