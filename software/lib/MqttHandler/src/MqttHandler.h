#ifndef MqttHandler_H // include guard
#define MqttHandler_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <StirrerConfig.h>

// Publishes stirrer state to "stirrer/<id>/<subtopic>".
class MqttHandler{
    public:
        MqttHandler(const char* stirrerId, StirrerConfig::Mqtt * mqttConfigPointer);
        void setup();
        // keeps the connection alive and reconnects when it was lost
        void loop();
        void publishStatus(const char* status);
        void publishIp();
    private:
        bool reconnect();
        void publish(const char* subTopic, const char* payload, bool retained);
        String stirrerId;
        StirrerConfig::Mqtt * mqttConfig;
        WiFiClient wifiClient;
        PubSubClient client;
        unsigned long lastReconnectAttempt;
};
#endif // MqttHandler_H
