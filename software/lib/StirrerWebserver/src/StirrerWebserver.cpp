#include <StirrerWebserver.h>
#include <Update.h>
#include <incbin.h>
#include <ArduinoJson.h>
#include <StirrerConfig.h>

#define DEBUG

INCTXT(StyleCss, "lib/StirrerWebserver/src/style.css");
INCTXT(IndexHtml, "lib/StirrerWebserver/src/index.html");
INCTXT(ConfigHtml, "lib/StirrerWebserver/src/config.html");

StirrerWebserver::StirrerWebserver(){
    this->myServer = new AsyncWebServer(80);
}

StirrerWebserver::~StirrerWebserver(){
    myServer->end();
    delete myServer;
}

void StirrerWebserver::setup(){
    /*return login*/
    myServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("/");
        #endif
        request->send(200, "text/html", (uint8_t *)gIndexHtmlData, gIndexHtmlSize);
    });
    /*return login*/
    myServer->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("/index.html");
        #endif
        request->send(200, "text/html", (uint8_t *)gIndexHtmlData, gIndexHtmlSize);
    });
    /*return style*/
    myServer->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("/style.css");
        #endif
        request->send(200, "text/html", (uint8_t *)gStyleCssData, gStyleCssSize);
    });
    /*return config*/
    myServer->on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("/config.html");
        #endif
        request->send(200, "text/html", (uint8_t *)gConfigHtmlData, gConfigHtmlSize);
    });
    /*set config*/
    myServer->on("/config.html", HTTP_POST, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("config recieved answer");
        #endif
        Serial.println(request->contentType());
        int params = request->params();
        #ifdef DEBUG
        Serial.printf("%d params sent in\n", params);
        for (int i = 0; i < params; i++)
        {
            const AsyncWebParameter *p = request->getParam(i);
            if (p->isPost())
            {
                Serial.printf("%s: %s \n", p->name().c_str(), p->value().c_str());
            }
            else
            {
                Serial.println("No post param");
                Serial.printf("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
            }
        }
        #endif

        StirrerConfig stirrerConfig;
        const AsyncWebParameter* p = request->getParam("wifiSsid", true);
        if (p == nullptr) {
            request->send(400, "text/plain", "Missing wifiSsid parameter");
            return;
        }
        stirrerConfig.wifi.ssid = p->value();

        p = request->getParam("wifiPwd", true);
        if (p == nullptr) {
            request->send(400, "text/plain", "Missing wifiPwd parameter");
            return;
        }
        stirrerConfig.wifi.password = p->value();

        p = request->getParam("mqttServerIp", true);
        if (p == nullptr) {
            request->send(400, "text/plain", "Missing mqttServerIp parameter");
            return;
        }
        stirrerConfig.mqtt.ip = p->value();

        p = request->getParam("mqttUser", true);
        if (p == nullptr) {
            request->send(400, "text/plain", "Missing mqttUser parameter");
            return;
        }
        stirrerConfig.mqtt.user = p->value();

        p = request->getParam("mqttPwd", true);
        if (p == nullptr) {
            request->send(400, "text/plain", "Missing mqttPwd parameter");
            return;
        }
        stirrerConfig.mqtt.password = p->value();

        stirrerConfig.write();
        JsonDocument doc = stirrerConfig.toJson();
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        serializeJson(doc, *response);
        request->send(response);
    });

    myServer->on("/api/config.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("/api/config.json");
        #endif
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        StirrerConfig stirrerConfig;
        stirrerConfig.load();
        JsonDocument doc = stirrerConfig.toJson();
        serializeJson(doc, *response);
        request->send(response);
    });

    myServer->begin();
}
