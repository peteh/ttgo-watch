#pragma once
#include <WiFi.h>
#include <mutex>
namespace twatch
{
    class WifiManager
    {
    public:
        static WifiManager &instance()
        {
            static WifiManager instance; // Guaranteed to be destroyed.
                                         // Instantiated on first use.
            return instance;
        }
        void init();

        bool connect();
        void connectAsync();
        void connectAsync(const char *ssid, const char *password);
        bool connect(const char *ssid, const char *password);
        bool connectAndStore(const char *ssid, const char *password);

        bool isEnabled()
        {
            return m_enabled;
        }

        void storeSettings();

        void disconnect();

        //WifiManager(WifiManager const &) = delete;
        //void operator=(WifiManager const &) = delete;

    private:
        WifiManager();
        //WifiManager(WifiManager const &);    // Don't Implement
        //void operator=(WifiManager const &); // Don't implement
        char m_ssid[128];
        char m_password[128];
        bool m_enabled;
        std::mutex m_mutexFile;

        static const long CONNECT_TIMEOUT = 5000;
    };
}