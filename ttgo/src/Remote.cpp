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
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    Log::info("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Log::info(".");
    }
    Log::infof("Connected, IP address: %s", WiFi.localIP().toString().c_str());
    m_mqttClient.setServer("192.168.2.135", 1883);
    reconnect();
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
    m_mqttClient.loop();
}

void Remote::send(float left, float right, bool honk)
{
    StaticJsonDocument<256> doc;
    doc["left"] = left;
    doc["right"] = right;
    doc["honk"] = honk;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    m_mqttClient.publish("remote", buffer, n);
}

void Remote::reconnect()
    {
        // Loop until we're reconnected
        while (!m_mqttClient.connected())
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