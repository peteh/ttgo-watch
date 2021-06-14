#include "Remote.h"

#include <Log.h>
#include <ArduinoJson.h>

//SSID of your network
char ssid[] = "Drop it like it's Hotspot";
//password of your WPA Network
char pass[] = "fortunabier";

Remote::Remote()
    : m_wifiClient(),
      m_mqttClient(m_wifiClient)
{
}
void Remote::init()
{
    m_mqttClient.setServer("192.168.2.135", 1883);
}

String Remote::macToStr(const uint8_t *mac)
{
    String result;
    for (int i = 0; i < 6; ++i)
    {
        result += String(mac[i], 16);
        if (i < 5)
            result += ':';
    }
    return result;
}
String Remote::composeClientID()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String clientId;
    clientId += "esp-";
    clientId += macToStr(mac);
    return clientId;
}

void Remote::loop()
{
    // will try one reconnect attempt if not connected
    reconnect();
    m_mqttClient.loop();
}

void Remote::send(float left, float right, bool honk)
{
    StaticJsonDocument<256> doc;
    doc["left"] = left;
    doc["right"] = right;
    doc["honk"] = honk;

    doc["aYL"] = 0;
    doc["aXR"] = 0;

    // forward
    if(left > 0 && right > 0)
    {
        doc["aXR"] = 1.0;
    }
    // backward
    if(left < 0 && right < 0)
    {
        doc["aXR"] = -1.0;
    }

    // left
    if(left < 0 && right > 0)
    {
        doc["aYL"] = 1.0;
    }
    // right
    if(left > 0 && right < 0)
    {
        doc["aYL"] = -1.0;
    }

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    m_mqttClient.publish("remote", buffer, n);
}

void Remote::reconnect()
{
    // Loop until we're reconnected
    if (!m_mqttClient.connected())
    {
        Log::info("Attempting MQTT connection...");

        String clientId = composeClientID();
        clientId += "-";
        clientId += String(micros() & 0xff, 16); // to randomise. sort of

        // Attempt to connect
        if (m_mqttClient.connect(clientId.c_str()))
        {
            Log::info("connected");
            // Once connected, publish an announcement...
            //client.publish(ROOT_TOPIC.c_str(), ("connected " + composeClientID()).c_str() , true );
            // ... and resubscribe
            // topic + clientID + in
            String subscribeTopic = "remote";
            m_mqttClient.subscribe(subscribeTopic.c_str());
            Log::infof("subscribed to: %s", subscribeTopic.c_str());
        }
        else
        {
            Log::errorf("failed, rc=%d", m_mqttClient.state());
            Log::errorf(" wifi=%d", WiFi.status());
            Log::error(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}