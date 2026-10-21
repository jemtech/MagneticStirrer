# MagneticStirrer

ESP32 firmware for a heated magnetic stirrer. It controls a 250 W / 230 V resistive heating plate and a 12 V brushed DC stirring motor. Both run in closed loop with a PID controller. You operate the stirrer with a rotary encoder and a 16x2 LCD, from a built-in web interface, or over MQTT.

## Features

- **Temperature control**: PID control of the heating plate via an SSR (slow 2 Hz PWM). The process value comes from either the plate NTC or an optional external NTC probe placed in the liquid. Each sensor has its own PID tuning.
- **Safety shutdown**: the heater is switched off and disabled whenever the plate NTC reads above the configured maximum (default 300 °C) or below -10 °C (sensor fault / disconnected). The plate sensor is always used for this check, even when regulating on the external probe.
- **Speed control**: PID control of the motor speed (RPM) via a MOSFET at 20 kHz PWM, with a non-blocking kick-start to get the motor spinning. Speed feedback comes from a Hall sensor (pin is a placeholder, not wired yet).
- **Local UI**: HD44780 16x2 LCD plus a push/turn rotary encoder.
- **Web interface**: status page, control API and WiFi/MQTT configuration form.
- **MQTT**: publishes status, IP, config, speed, temperature and PID state as retained messages.
- **WiFi with fallback AP**: if no SSID is configured or the connection fails, the device opens its own access point so it can be configured.
- **OTA updates** via ArduinoOTA.
- **Persistent config** stored in ESP32 NVS (`Preferences`).

## Hardware

| Function            | ESP32 pin(s)                              |
|---------------------|-------------------------------------------|
| Plate NTC (100k, B3950, 22k divider) | 34                       |
| External NTC probe  | 35                                        |
| Heater SSR          | 25                                        |
| Motor MOSFET        | 27                                        |
| Hall sensor (RPM)   | 13 (placeholder)                          |
| Encoder CLK / DT / SW | 32 / 33 / 14                            |
| LCD RS / EN / D4–D7 | 19 / 23 / 18, 17, 16, 15                  |

Target board: AZ-Delivery ESP32 DevKit v4.

## Operation

### Encoder and display

- **Turn**: change the value on the current page
- **Short press**: next page
- **Long press** (≥ 800 ms): save the config to flash

Pages: status, target temperature, target RPM, max plate temperature, sensor mode (plate/external), heater on/off, motor on/off.

### Network

On boot the stirrer joins the configured WiFi. Its hostname is `esp32stirrer_<id>`, so it can be reached as `esp32stirrer_<id>.lan` via the router's DNS and is announced via mDNS. If it can't connect, it opens an access point named `Stirrer-AP-<MAC>`. Connect to it and open `http://192.168.4.1/config.html` to enter WiFi and MQTT settings.

### Web interface and API

| Method | Path                | Description                                           |
|--------|---------------------|-------------------------------------------------------|
| GET    | `/`, `/index.html`  | Status / control page                                 |
| GET    | `/config.html`      | WiFi and MQTT configuration form                      |
| POST   | `/config.html`      | Save WiFi and MQTT settings (form data)               |
| GET    | `/api/status.json`  | Measured values, setpoints and PID parameters         |
| GET    | `/api/config.json`  | Full stored config                                    |
| POST   | `/api/control.json` | Update control settings (JSON body, all fields optional) |

Example control request:

```bash
curl -X POST http://esp32stirrer_0.lan/api/control.json \
  -H 'Content-Type: application/json' \
  -d '{"targetTemp": 60, "heaterEnabled": true, "targetRpm": 400, "motorEnabled": true}'
```

Accepted fields: `targetTemp`, `tempPlateMax`, `targetRpm`, `maxRpm`, `useExtSensor`, `heaterEnabled`, `motorEnabled`, `pidPlate`, `pidExt`, `pidMotor` (each PID as `{"kp":…, "ki":…, "kd":…}`).

### MQTT

When an MQTT broker IP is configured, the stirrer connects on port 1883 and publishes retained messages under `stirrer/<id>/`:

| Topic          | Payload                                                         |
|----------------|-----------------------------------------------------------------|
| `status`       | `online` / `offline` (last will)                                |
| `ip`           | Current IP address                                              |
| `config`       | Config as JSON, without passwords                               |
| `speed`        | Measured speed in RPM                                           |
| `temperature`  | Controlled temperature in °C (external probe if active, otherwise plate) |
| `pid`          | `{"heater":{"enabled","setpoint","output"},"motor":{…}}`, output in % |

Values are only re-published when they change. The full state is re-sent after every reconnect.

## Software

The project is a [PlatformIO](https://platformio.org/) project in `software/` using the Arduino framework.

### Build and flash

```bash
cd software
pio run -e az-delivery-devkit-v4 -t upload
pio device monitor        # 115200 baud
```

To update over the air, switch `upload_protocol` to `espota` in `platformio.ini` and set `upload_port` to the device IP. Serial debug output is enabled by `-DDEBUG` in `build_flags`.

### Tests

The unit tests run on the host, no board required. Arduino, Preferences, PID and the display are replaced by mocks in `test/mocks`.

```bash
cd software
pio test -e native
```

## Safety notice

This device switches mains voltage (230 V) and drives a heating element that can reach several hundred °C. Building and operating it is at your own risk. Make sure wiring, insulation, fusing and enclosure meet the applicable safety standards, and never leave the stirrer heating unattended.
