#include "StirrerWebserver.h"
#include <Update.h>
#include <incbin.h>
#include <ArduinoJson.h>


INCTXT(StyleCss,   "lib/StirrerWebserver/src/style.css");
INCTXT(IndexHtml,  "lib/StirrerWebserver/src/index.html");
INCTXT(ConfigHtml, "lib/StirrerWebserver/src/config.html");

StirrerWebserver::StirrerWebserver() {
    this->myServer = new AsyncWebServer(80);
}

StirrerWebserver::~StirrerWebserver() {
    myServer->end();
    delete myServer;
}

void StirrerWebserver::setup(StirrerConfig* config, StirrerControl* control) {
    _config  = config;
    _control = control;

    // ── Statische Files ──────────────────────────────────────
    myServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", (uint8_t*)gIndexHtmlData, gIndexHtmlSize);
    });
    myServer->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", (uint8_t*)gIndexHtmlData, gIndexHtmlSize);
    });
    myServer->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/css", (uint8_t*)gStyleCssData, gStyleCssSize);
    });
    myServer->on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", (uint8_t*)gConfigHtmlData, gConfigHtmlSize);
    });

    // ── Config speichern (WiFi + MQTT) ───────────────────────
    myServer->on("/config.html", HTTP_POST, [this](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("POST /config.html");
        #endif
        WifiConfig oldWifi = _config->wifi;
        MqttConfig oldMqtt = _config->mqtt;
        const char* missing = _config->applyForm([request](const char* name, String& value) {
            const AsyncWebParameter* p = request->getParam(name, true);
            if (p == nullptr) return false;
            value = p->value();
            return true;
        });
        if (missing != nullptr) {
            // applyForm may have updated some fields already
            _config->wifi = oldWifi;
            _config->mqtt = oldMqtt;
            request->send(400, "text/plain", String("Missing ") + missing + " parameter");
            return;
        }
        _config->write();

        AsyncResponseStream* response = request->beginResponseStream("application/json");
        JsonDocument doc = _config->toPublicJson();
        serializeJson(doc, *response);
        request->send(response);
    });

    // ── Config lesen ─────────────────────────────────────────
    myServer->on("/api/config.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("GET /api/config.json");
        #endif
        AsyncResponseStream* response = request->beginResponseStream("application/json");
        JsonDocument doc = _config->toJson();
        serializeJson(doc, *response);
        request->send(response);
    });

    // ── Status lesen (Echtmesswerte) ─────────────────────────
    myServer->on("/api/status.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
        #ifdef DEBUG
        Serial.println("GET /api/status.json");
        #endif
        AsyncResponseStream* response = request->beginResponseStream("application/json");
        JsonDocument doc;

        // Messwerte
        doc["tempPlate"]      = _control->getTempPlate();
        doc["tempExt"]        = _control->getTempExt();
        doc["extConnected"]   = _control->isExtConnected();
        doc["pidOutput"]      = _control->getPidOutput();
        doc["pidMotorOutput"] = _control->getPidMotorOutput();
        doc["currentRpm"]     = _control->getCurrentRpm();

        // Sollwerte
        doc["targetTemp"]     = _config->control.targetTemp;
        doc["tempPlateMax"]   = _config->control.tempPlateMax;
        doc["targetRpm"]      = _config->control.targetRpm;
        doc["maxRpm"]         = _config->control.maxRpm;
        doc["useExtSensor"]   = _config->control.useExtSensor;
        doc["heaterEnabled"]  = _config->control.heaterEnabled;
        doc["motorEnabled"]   = _config->control.motorEnabled;

        JsonObject pidPlate = doc["pidPlate"].to<JsonObject>();
        pidPlate["kp"] = _config->control.pidPlate.kp;
        pidPlate["ki"] = _config->control.pidPlate.ki;
        pidPlate["kd"] = _config->control.pidPlate.kd;

        JsonObject pidExt = doc["pidExt"].to<JsonObject>();
        pidExt["kp"] = _config->control.pidExt.kp;
        pidExt["ki"] = _config->control.pidExt.ki;
        pidExt["kd"] = _config->control.pidExt.kd;

        JsonObject pidMotor = doc["pidMotor"].to<JsonObject>();
        pidMotor["kp"] = _config->control.pidMotor.kp;
        pidMotor["ki"] = _config->control.pidMotor.ki;
        pidMotor["kd"] = _config->control.pidMotor.kd;

        serializeJson(doc, *response);
        request->send(response);
    });

    // ── Control POST (JSON Body) ──────────────────────────────
    myServer->onRequestBody([this](AsyncWebServerRequest *request, uint8_t *data,
                                   size_t len, size_t index, size_t total) {
        if (request->url() == "/api/control.json" && request->method() == HTTP_POST) {
            #ifdef DEBUG
            Serial.println("POST /api/control.json");
            #endif
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);
            if (err) {
                request->send(400, "application/json", "{\"error\":\"invalid json\"}");
                return;
            }
            if (!doc["targetTemp"].isNull())
                _config->control.targetTemp    = doc["targetTemp"].as<float>();
            if (!doc["tempPlateMax"].isNull())
                _config->control.tempPlateMax  = doc["tempPlateMax"].as<float>();
            if (!doc["targetRpm"].isNull())
                _config->control.targetRpm     = doc["targetRpm"].as<float>();
            if (!doc["maxRpm"].isNull())
                _config->control.maxRpm        = doc["maxRpm"].as<float>();
            if (!doc["useExtSensor"].isNull())
                _config->control.useExtSensor  = doc["useExtSensor"].as<bool>();
            if (!doc["heaterEnabled"].isNull())
                _config->control.heaterEnabled = doc["heaterEnabled"].as<bool>();
            if (!doc["motorEnabled"].isNull())
                _config->control.motorEnabled  = doc["motorEnabled"].as<bool>();
            if (!doc["pidPlate"].isNull()) {
                _config->control.pidPlate.kp = doc["pidPlate"]["kp"].as<double>();
                _config->control.pidPlate.ki = doc["pidPlate"]["ki"].as<double>();
                _config->control.pidPlate.kd = doc["pidPlate"]["kd"].as<double>();
            }
            if (!doc["pidExt"].isNull()) {
                _config->control.pidExt.kp = doc["pidExt"]["kp"].as<double>();
                _config->control.pidExt.ki = doc["pidExt"]["ki"].as<double>();
                _config->control.pidExt.kd = doc["pidExt"]["kd"].as<double>();
            }
            if (!doc["pidMotor"].isNull()) {
                _config->control.pidMotor.kp = doc["pidMotor"]["kp"].as<double>();
                _config->control.pidMotor.ki = doc["pidMotor"]["ki"].as<double>();
                _config->control.pidMotor.kd = doc["pidMotor"]["kd"].as<double>();
            }
            _config->write();

            AsyncResponseStream* response = request->beginResponseStream("application/json");
            JsonDocument resp;
            resp["ok"] = true;
            serializeJson(resp, *response);
            request->send(response);
        }
    });

    myServer->begin();
}
