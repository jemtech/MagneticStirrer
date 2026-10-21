#include <MqttHandler.h>
#include <WiFi.h>

#define DEBUG
#define MQTT_PORT 1883
#define RECONNECT_INTERVAL 5000 // ms

MqttHandler::MqttHandler(const char* stirrerId, StirrerConfig::Mqtt * mqttConfigPointer)
    : stirrerId(stirrerId), mqttConfig(mqttConfigPointer), client(wifiClient), lastReconnectAttempt(0){
}

void MqttHandler::setup(){
    IPAddress serverIp;
    if (!serverIp.fromString(mqttConfig->ip)) {
        #ifdef DEBUG
        Serial.printf("invalid mqtt server ip:\"%s\"\n", mqttConfig->ip.c_str());
        #endif
        return;
    }
    client.setServer(serverIp, MQTT_PORT);
    reconnect();
}

bool MqttHandler::reconnect(){
    lastReconnectAttempt = millis();
    if (WiFi.status() != WL_CONNECTED) return false;

    String clientId = "esp32stirrer_" + stirrerId;
    String willTopic = "stirrer/" + stirrerId + "/status";
    const char* user = mqttConfig->user.length() > 0 ? mqttConfig->user.c_str() : NULL;
    const char* password = mqttConfig->password.length() > 0 ? mqttConfig->password.c_str() : NULL;
    bool connected = client.connect(clientId.c_str(), user, password,
                                    willTopic.c_str(), 0, true, "offline");
    #ifdef DEBUG
    if (connected) {
        Serial.println("mqtt connected");
    } else {
        Serial.printf("mqtt connect failed, state %d\n", client.state());
    }
    #endif
    return connected;
}

void MqttHandler::loop(){
    if (client.connected()) {
        client.loop();
    } else if (millis() - lastReconnectAttempt >= RECONNECT_INTERVAL) {
        reconnect();
    }
}

void MqttHandler::publish(const char* subTopic, const char* payload, bool retained){
    if (!client.connected()) return;
    String topic = "stirrer/" + stirrerId + "/" + subTopic;
    client.publish(topic.c_str(), payload, retained);
}

void MqttHandler::publishStatus(const char* status){
    publish("status", status, true);
}

void MqttHandler::publishIp(){
    publish("ip", WiFi.localIP().toString().c_str(), true);
}
