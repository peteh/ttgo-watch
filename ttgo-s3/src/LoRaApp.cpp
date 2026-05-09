#include "LoRaApp.h"
#include "esp_log.h"
/**
 * T-Watch S3 — Minimal LoRaWAN OTAA Sensor Node for ChirpStack
 * ============================================================
 * Libraries required:
 *   - LilyGoLib  (from https://github.com/Xinyuan-LilyGO/LilyGoLib)
 *     + LilyGoLib-ThirdParty (XPowersLib, etc.)
 *   - RadioLib   (install via Arduino Library Manager)
 *
 * Arduino ESP32 core: >= v3.3.0-alpha1
 *   URL: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json
 *
 * Board selection in Arduino IDE:
 *   Tools > Board > ESP32S3 Dev Module
 *   Tools > PSRAM > OPI PSRAM
 *   Tools > USB Mode > USB-OTG (TinyUSB)
 *
 * ChirpStack device setup:
 *   - LoRaWAN MAC version: 1.0.4
 *   - Regional parameters: RP002-1.0.3 (or latest)
 *   - Activation: OTAA
 *   - DevEUI: copy from ChirpStack, paste REVERSED into joinEUI array below
 *   - AppKey: copy as-is (MSB order) into appKey array below
 *   - JoinEUI/AppEUI: use 0000000000000000 (ChirpStack doesn't require it)
 */

// =========================================================
// YOUR CREDENTIALS — fill these in before flashing
// =========================================================

// DevEUI from ChirpStack, reversed (LSB first).
// ChirpStack shows: 01 02 03 04 05 06 07 08
// Enter here as:   { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 }

// JoinEUI / AppEUI — leave all zeros for ChirpStack
static const uint64_t joinEUI = 0x0000000000000000ULL;

// AppKey from ChirpStack, MSB order (copy exactly as shown)
static const uint8_t appKey[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

// NwkKey = AppKey for LoRaWAN 1.0.x
static const uint8_t nwkKey[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

// How often to send an uplink (milliseconds). Keep >= 60000 for fair use.
#define TX_INTERVAL_MS 60000

// =========================================================
// Hardware pin definitions — T-Watch S3 (SX1262 version)
// =========================================================
#define RADIO_SCLK 3
#define RADIO_MISO 4
#define RADIO_MOSI 1
#define RADIO_CS 5
#define RADIO_DIO1 9
#define RADIO_RST 8
#define RADIO_BUSY 7

// AXP2101 PMU — I2C bus
#define PMU_SDA 10
#define PMU_SCL 11
#define PMU_ADDR 0x34

// =========================================================
// Includes
// =========================================================
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>

// XPowersLib is bundled with LilyGoLib-ThirdParty
#include <XPowersLib.h>

// =========================================================
// Globals
// =========================================================
SPIClass loraSPI(FSPI);
extern SX1262 radio;
// SX1262   radio = new Module(RADIO_CS, RADIO_DIO1, RADIO_RST, RADIO_BUSY, loraSPI);

LoRaWANNode node(&radio, &EU868); // Change to your region: US915, AU915, AS923, etc.

XPowersAXP2101 pmu;

uint64_t generateDevEUI()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    return ((uint64_t)mac[0] << 56) |
           ((uint64_t)mac[1] << 48) |
           ((uint64_t)mac[2] << 40) |
           ((uint64_t)0xFF << 32) |
           ((uint64_t)0xFE << 24) |
           ((uint64_t)mac[3] << 16) |
           ((uint64_t)mac[4] << 8) |
           ((uint64_t)mac[5]);
}

bool initPMU()
{
    Wire.begin(PMU_SDA, PMU_SCL);

    if (!pmu.begin(Wire, PMU_ADDR, PMU_SDA, PMU_SCL))
    {
        Serial.println("[PMU] AXP2101 not found!");
        return false;
    }
    Serial.println("[PMU] AXP2101 OK");

    // Power rails required for the T-Watch S3:
    //   ALDO2 powers the SX1262 LoRa module
    //   ALDO3 powers the display backlight / touch controller
    //   DC1   is the main 3.3 V rail
    // We only strictly need ALDO2 for LoRa, but enabling the others
    // avoids issues if you later want to use the display.

    pmu.setDC1Voltage(3300);
    pmu.enableDC1();

    pmu.setALDO2Voltage(3300); // LoRa SX1262 power
    pmu.enableALDO2();

    pmu.setALDO3Voltage(3300); // Display / touch
    pmu.enableALDO3();

    // Let the rails stabilise before touching SPI
    delay(100);
    return true;
}

void printHex(const char *label, const uint8_t *buf, size_t len)
{
    Serial.print(label);
    for (size_t i = 0; i < len; i++)
        Serial.printf("%02X", buf[i]);
    Serial.println();
}

void LoRaApp::btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;
        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
        ESP_LOGI("LoRaApp", "Button :%d", cnt);
        sendUplink(EventType::BUTTON);
    }
}

void LoRaApp::setup()
{
    Serial.begin(115200);
    delay(2000); // Give time for serial monitor to connect
    Serial.println("\n[T-Watch S3] LoRaWAN OTAA node starting...");

    uint64_t devEUI = generateDevEUI();

    // 1. Power on peripherals via PMU
    if (!initPMU())
    {
        Serial.println("[FATAL] PMU init failed — halting.");
        while (true)
            delay(1000);
    }

    // 2. Start SPI for SX1262
    loraSPI.begin(RADIO_SCLK, RADIO_MISO, RADIO_MOSI, RADIO_CS);

    // 3. Initialise SX1262
    Serial.print("[Radio] Initialising SX1262... ");
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.printf("FAILED (code %d)\n", state);
        Serial.println("[HINT] Check PMU ALDO2 is on and SPI pins are correct.");
        while (true)
            delay(1000);
    }
    Serial.println("OK");

    // 4. Configure LoRaWAN node
    // Persistent session storage (survives deep sleep):
    //   node.setDeviceAddress(...)  — not needed for OTAA
    // DevNonce counter is stored in NVS automatically by RadioLib.

    // print all parameters for chirpstack - so we can just copy and paste
    Serial.println("[LoRaWAN] Configuring node with OTAA credentials...");
    Serial.printf("[LoRaWAN] Join EUI: %016llX\n", joinEUI);
    Serial.printf("[LoRaWAN] Dev EUI: %016llX\n", devEUI);

    printHex("[LoRaWAN] App Key: ", appKey, 16);
    printHex("[LoRaWAN] Nwk Key: ", nwkKey, 16);

    node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);

    // 5. Join the network
    Serial.println("[LoRaWAN] Joining network (OTAA)...");
    state = node.activateOTAA();
    if (state != RADIOLIB_LORAWAN_SESSION_RESTORED &&
        state != RADIOLIB_LORAWAN_NEW_SESSION &&
        state != RADIOLIB_ERR_NONE)
    {
        Serial.printf("[LoRaWAN] Join failed (code %d)\n", state);
        Serial.println("[HINT] Check DevEUI byte order and AppKey in ChirpStack.");
        while (true)
            delay(1000);
    }
    Serial.printf("[LoRaWAN] Joined! DevAddr: 0x%08X\n", node.getDevAddr());
    m_joined = true;
    m_lastTx = millis();

    lv_obj_t *label = lv_label_create(lv_screen_active()); /*Add a label the current screen*/
    lv_label_set_text(label, "Hello World");               /*Set label text*/
    lv_obj_center(label);                                  /*Set center alignment*/

    m_btn = lv_button_create(lv_screen_active()); /*Add a button the current screen*/
    lv_obj_set_size(m_btn, 120, 50);              /*Set its size*/

    lv_obj_add_event_cb(m_btn, [](lv_event_t *e)
                        {
        auto *app = static_cast<LoRaApp *>(lv_event_get_user_data(e));
        app->btn_event_cb(e); }, LV_EVENT_ALL, this);                 /*Assign a callback to the button*/
    lv_obj_align_to(m_btn, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); /*Set the label to it and align it in the center below the label*/

    lv_obj_t *btn_label = lv_label_create(m_btn); /*Add a label to the button*/
    lv_label_set_text(btn_label, "Button");       /*Set the labels text*/
    lv_obj_center(btn_label);
}



void LoRaApp::sendUplink(EventType eventType)
{
    // --- Build a minimal payload ---
    // Example: 2-byte counter (big-endian) + battery voltage (2 bytes, mV)
    static uint16_t counter = 0;
    counter++;

    uint16_t battMV = (uint16_t)pmu.getBattVoltage();

    uint8_t payload[5];
    payload[0] = static_cast<uint8_t>(eventType); // 1 byte for event type
    payload[1] = (counter >> 8) & 0xFF;
    payload[2] = counter & 0xFF;
    payload[3] = (battMV >> 8) & 0xFF;
    payload[4] = battMV & 0xFF;


    Serial.printf("[TX] counter=%u  batt=%u mV\n", counter, battMV);

    // Send on port 1, unconfirmed
    int16_t state = node.sendReceive(payload, sizeof(payload), 1, true);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("[TX] Uplink sent, no downlink.");
    }
    else if (state > 0)
    {
        // Positive return = downlink received on that port
        Serial.println("[TX] Uplink sent, downlink received.");
        uint8_t downData[256];
        // TODO: make download work
        // size_t  downLen = node.getDownlinkDataLen();
        // node.getDownlinkData(downData, downLen);
        // Serial.printf("[RX] Downlink on port %d, %u bytes\n", state, downLen);
    }
    else
    {
        Serial.printf("[TX] Error (code %d)\n", state);
    }
}

void LoRaApp::loop()
{
    if (!m_joined)
        return;

    uint32_t now = millis();
    if (now - m_lastTx < TX_INTERVAL_MS)
    {
        yield(); // Let the system do background work (e.g. LoRaWAN state machine)
        return;
    }
    m_lastTx = now;
    sendUplink(EventType::STATUS);
}