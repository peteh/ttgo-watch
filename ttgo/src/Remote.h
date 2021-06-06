#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

class Remote
{
public:
    Remote();
    void init();
    void reconnect();
    void send(float left, float right, bool honk);
    void loop();

private:
    String composeClientID();
    String macToStr(const uint8_t *mac);

    WiFiClient m_wifiClient;
    PubSubClient m_mqttClient;
};
