#include <MqttHandler.h>
#include <WiFi.h>

#define MQTT_PORT 1883
#define RECONNECT_INTERVAL 5000 // ms
#define TEMPERATURE_THRESHOLD 0.1 // °C
#define CONFIG_BUFFER_SIZE 1024 // config json incl. control + pid values

MqttHandler::MqttHandler(const char* stirrerId, StirrerConfig * stirrerConfigPointer)
    : stirrerId(stirrerId), stirrerConfig(stirrerConfigPointer), client(wifiClient),
      lastReconnectAttempt(0), status(), speed(0), hasTemperature(false), temperature(0){
    client.setBufferSize(CONFIG_BUFFER_SIZE);
}

void MqttHandler::setup(){
    IPAddress serverIp;
    if (!serverIp.fromString(stirrerConfig->mqtt.ip)) {
        #ifdef DEBUG
        Serial.printf("invalid mqtt server ip:\"%s\"\n", stirrerConfig->mqtt.ip.c_str());
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
    const char* user = stirrerConfig->mqtt.user.length() > 0 ? stirrerConfig->mqtt.user.c_str() : NULL;
    const char* password = stirrerConfig->mqtt.password.length() > 0 ? stirrerConfig->mqtt.password.c_str() : NULL;
    bool connected = client.connect(clientId.c_str(), user, password,
                                    willTopic.c_str(), 0, true, "offline");
    #ifdef DEBUG
    if (connected) {
        Serial.println("mqtt connected");
    } else {
        Serial.printf("mqtt connect failed, state %d\n", client.state());
    }
    #endif
    if (connected) publishState();
    return connected;
}

// state that has to be on the broker after every (re)connect
void MqttHandler::publishState(){
    // replaces the retained "offline" will from a previous connection
    if (status.length() > 0) {
        publish("status", status.c_str(), true);
    }
    publishIp();
    publishConfig();
    publish("speed", String(speed).c_str(), true);
    if (hasTemperature) {
        publish("temperature", String(temperature, 1).c_str(), true);
    }
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
    this->status = status;
    publish("status", status, true);
}

void MqttHandler::publishIp(){
    publish("ip", WiFi.localIP().toString().c_str(), true);
}

void MqttHandler::publishConfig(){
    JsonDocument doc = stirrerConfig->toPublicJson();
    String payload;
    serializeJson(doc, payload);
    publish("config", payload.c_str(), true);
}

void MqttHandler::publishSpeed(int speed){
    if (speed == this->speed) return;
    this->speed = speed;
    publish("speed", String(speed).c_str(), true);
}

void MqttHandler::publishTemperature(float temperature){
    if (hasTemperature && fabsf(temperature - this->temperature) < TEMPERATURE_THRESHOLD) return;
    hasTemperature = true;
    this->temperature = temperature;
    publish("temperature", String(temperature, 1).c_str(), true);
}
