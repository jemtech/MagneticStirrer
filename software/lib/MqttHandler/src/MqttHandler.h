#ifndef MqttHandler_H // include guard
#define MqttHandler_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <StirrerConfig.h>

// Publishes stirrer state to "stirrer/<id>/<subtopic>".
// All topics are retained, so subscribers get the last state immediately.
// Config, speed and temperature are (re)published on every (re)connect.
class MqttHandler{
    public:
        MqttHandler(const char* stirrerId, StirrerConfig * stirrerConfigPointer);
        void setup();
        // keeps the connection alive and reconnects when it was lost
        void loop();
        void publishStatus(const char* status);
        void publishIp();
        // config without passwords as json
        void publishConfig();
        // stirrer pwm value 0-255
        void publishSpeed(int speed);
        // only publishes when it changed by at least TEMPERATURE_THRESHOLD
        void publishTemperature(float temperature);
    private:
        bool reconnect();
        void publishState();
        void publish(const char* subTopic, const char* payload, bool retained);
        String stirrerId;
        StirrerConfig * stirrerConfig;
        WiFiClient wifiClient;
        PubSubClient client;
        unsigned long lastReconnectAttempt;
        int speed;
        bool hasTemperature;
        float temperature;
};
#endif // MqttHandler_H
