#include "WifiManager.h"
#include <Log.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
namespace twatch
{

    WifiManager::WifiManager()
        : m_ssid(""),
          m_password(""),
          m_enabled(false)
    {
    }

    void WifiManager::connectAsync()
    {
        connectAsync(m_ssid, m_password);
    }

    void WifiManager::connectAsync(const char *ssid, const char *password)
    {
        Log::infof("SSID: %s", ssid);
        Log::infof("Pass: %s", password);
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        WiFi.setAutoReconnect(true);
        Log::info("Connecting");
    }

    bool WifiManager::connect()
    {
        return connect(m_ssid, m_password);
    }

    bool WifiManager::connect(const char *ssid, const char *password)
    {
        connectAsync(ssid, password);
        long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start <= CONNECT_TIMEOUT)
        {
            delay(500);
            Log::infof("Wifi Status: %d", WiFi.status());
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            WiFi.disconnect();
            return false;
        }
        Log::infof("Connected, IP address: %s", WiFi.localIP().toString().c_str());
        strncpy(m_ssid, ssid, sizeof(m_ssid));
        strncpy(m_password, password, sizeof(m_password));
        m_enabled = true;
        return true;
    }

    bool WifiManager::connectAndStore(const char *ssid, const char *password)
    {
        if(!connect(ssid, password))
        {
            Log::debug("Failed to connect to wifi, not storing settings");
            return false;
        }
        storeSettings();
        return true;
    }

    void WifiManager::disconnect()
    {
        WiFi.disconnect();
        m_enabled = false;
        storeSettings();
    }

    void WifiManager::init()
    {
        std::unique_lock<std::mutex> lock(m_mutexFile);
        fs::File configFile = SPIFFS.open("/wifiSettings.cfg", "r");
        if (!configFile)
        {
            Log::info("Failed to open config file");
            return;
        }

        size_t size = configFile.size();
        Log::infof("Config size: %d", size);
        if (size > 256)
        {
            Log::error("Config file size is too large");
            return;
        }

        // Allocate the memory pool on the stack.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<256> doc;
        // Deserialize the JSON document
        DeserializationError jsonError = deserializeJson(doc, configFile);
        configFile.close();
        if (jsonError)
        {
            Log::error("Failed to read file, using default configuration");
            return;
        }

        const char *ssid = doc["ssid"] | "";
        const char *password = doc["password"] | "";
        strncpy(m_ssid, ssid, sizeof(m_ssid));
        strncpy(m_password, password, sizeof(m_password));

        m_enabled = doc["enabled"] | false;

        Log::debugf("Loaded SSID: %s", m_ssid);
        Log::debugf("Loaded password: %s", m_password);
        Log::debugf("WiFi enabled: %d", m_enabled);
        Log::debug("Finished loading config");

        if(isEnabled())
        {
            connectAsync();
        }
    }

    void WifiManager::storeSettings()
    {
        std::unique_lock<std::mutex> lock(m_mutexFile);
        fs::File configFile = SPIFFS.open("/wifiSettings.cfg", "w");
        if (!configFile)
        {
            Log::info("Failed to open config file");
            return;
        }

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<256> doc;

        // Set the values in the document
        doc["ssid"] = m_ssid;
        doc["password"] = m_password;
        doc["enabled"] = m_enabled;

        // Serialize JSON to file
        if (serializeJson(doc, configFile) == 0)
        {
            Log::error("Failed to write to file");
        }

        // Close the file
        configFile.close();
        Log::debug("Saved WifiSettings file");
    }
}