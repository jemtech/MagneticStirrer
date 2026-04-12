#ifndef StirrerWebserver_H // include guard
#define StirrerWebserver_H

#include <ESPAsyncWebServer.h>

class StirrerWebserver{
    public:
        StirrerWebserver();
        ~StirrerWebserver();
        void setup();
    private:
        AsyncWebServer* myServer;
};
#endif // StirrerWebserver_H