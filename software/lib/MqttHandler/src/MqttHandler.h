#ifndef MqttHandler_H // include guard
#define MqttHandler_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <StirrerConfig.h>

// Publishes stirrer state to "stirrer/<id>/<subtopic>".
// All topics are retained, so subscribers get the last state immediately.
// Status, ip, config, speed, temperature and pid are (re)published on every (re)connect.
class MqttHandler{
    public:
        MqttHandler(const char* stirrerId, StirrerConfig * stirrerConfigPointer);
        void setup();
        // keeps the connection alive and reconnects when it was lost
        void loop();
        void publishStatus(const char* status);
        void publishIp();
        // config without passwords as json, only publishes when it changed
        // (feedback for changes made via encoder, webserver or safety shutdown)
        void publishConfig();
        // measured stirrer speed in rpm, only publishes when it changed
        void publishSpeed(int speed);
        // only publishes when it changed by at least TEMPERATURE_THRESHOLD
        void publishTemperature(float temperature);
        // pid state as json, outputs in % (rounded), only publishes when it changed
        void publishPid(bool heaterEnabled, float heaterSetpoint, float heaterOutput,
                        bool motorEnabled, float motorSetpoint, float motorOutput);
    private:
        bool reconnect();
        void publishState();
        void publish(const char* subTopic, const char* payload, bool retained);
        String stirrerId;
        StirrerConfig * stirrerConfig;
        WiFiClient wifiClient;
        PubSubClient client;
        unsigned long lastReconnectAttempt;
        String status;
        int speed;
        bool hasTemperature;
        float temperature;
        String config;
        String pid;
};
#endif // MqttHandler_H
