#include "voicepipeline.h"
#include <LilyGoLib.h>

void VoicePipeline::init()
{
    if (HA_USE_SSL)
    {
        m_ws.beginSSL(HA_HOST, HA_PORT, "/api/websocket");
    }
    else
    {
        m_ws.begin(HA_HOST, HA_PORT, "/api/websocket");
    }
    
    m_ws.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                 { this->onWSEvent(type, payload, length); });

    m_ws.setReconnectInterval(3000);
    m_audio.setVolume(21);         // 0..21
}

void VoicePipeline::transitionToState(PipelineState newState)
{
    m_state = newState;
    m_stateChangeTS = millis();
}

void VoicePipeline::onWSEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        m_authed = false;
        Serial.println(F("[WS] Disconnected"));
        break;
    case WStype_CONNECTED:
        Serial.println(F("[WS] Connected"));
        break;
    case WStype_TEXT:
    {
        String txt = String((const char *)payload, length);
        Serial.print(F("[WS] <= "));
        Serial.println(txt);

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, (const char *)payload, length);

        if (err)
        {
            Serial.print(F("[WS] JSON parse error: "));
            Serial.println(err.c_str());
            return;
        }

        if (doc["type"] == nullptr)
        {
            Serial.println(F("[WS] No type field"));
            return;
        }

        if (doc["type"] == "auth_required")
        {
            m_authed = false;
            String auth = String("{\"type\":\"auth\",\"access_token\":\"") + HA_TOKEN + "\"}";
            m_ws.sendTXT(auth);
            Serial.println(F("[WS] => auth token"));
            return;
        }

        if (doc["type"] == "auth_ok")
        {
            m_authed = true;
            Serial.println(F("[WS] Auth OK"));
            return;
        }
        // EVENT HANDLING

        if (doc["type"] == "event")
        {
            if (doc["event"] == nullptr || doc["event"]["type"] == nullptr)
            {
                Serial.println(F("[WS] No event_type field"));
                return;
            }

            String event_type = doc["event"]["type"].as<String>();
            Serial.print(F("[WS] Event type: "));
            Serial.println(event_type);
            if (event_type == "run-start")
            {
                Serial.println(F("[WS] STT start acknowledged by HA"));
                // HA has acknoweledged the start of the run
                m_stt_binary_handler_id = doc["event"]["data"]["runner_data"]["stt_binary_handler_id"].as<uint8_t>();
                Serial.println(F("[WS] STT binary handler id: "));
                Serial.println(m_stt_binary_handler_id);
                Serial.println(F("[WS] STT start acknowledged by HA"));
                return;
            }

            if (event_type == "stt-start")
            {
                Serial.println(F("[WS] STT start acknowledged by HA, we can send mic data now"));
                // HA has acknowledged start of STT; we can start sending mic data
                transitionToState(RECORDING);
                return;
            }

            if (event_type == "run-end")
            {
                Serial.println(F("[WS] HA ended the run, getting back to IDLE"));
                // HA has acknowledged end of input audio; we can stop sending mic data
                transitionToState(IDLE);
                return;
            }

            if (event_type == "tts-end")
            {
                m_ttsUrl = (HA_USE_SSL ? String("https") : String("http")) 
                + "://"
                + HA_HOST 
                + ":" 
                + HA_PORT 
                + doc["event"]["data"]["tts_output"]["url"].as<String>();
                transitionToState(PLAYING);
                Serial.print(F("[WS] TTS URL: "));
                Serial.println(m_ttsUrl);
            }
        }
    }
    break;
    default:
        break;
    }
}

void VoicePipeline::startRecording()
{
    if (m_state != IDLE)
    {
        Serial.println(F("[VP] Cannot start recording; not idle"));
        return;
    }

    if (!m_authed)
    {
        Serial.println(F("[VP] Cannot start recording; not authed"));
        return;
    }
    transitionToState(STARTING);
    m_runId++;
    String start = String("{") +
                   "\"id\":" + String(m_runId) + "," +
                   "\"type\":\"assist_pipeline/run\"," +
                   "\"start_stage\":\"stt\"," +
                   "\"end_stage\":\"tts\"," +
                   "\"input\":{\"sample_rate\":" + String(SAMPLE_RATE) + "}" +
                   "}";
    m_ws.sendTXT(start);

    Serial.print(F("[VP] => start run: "));
    Serial.println(start);
}

void VoicePipeline::stopRecording()
{
    if (m_state != RECORDING)
    {
        Serial.println(F("[VP] Cannot stop recording; not recording"));
        return;
    }

    transitionToState(STOPPING);

    // Signal end of input audio
    m_ws.sendBIN(&m_stt_binary_handler_id, 1);
}

void VoicePipeline::loop()
{
    m_ws.loop();
    uint32_t sample_rate = MIC_I2S_SAMPLE_RATE;
    uint16_t sample_width = 16;
    uint16_t num_channels = 1;

    int8_t mic_buf[SAMPLES_PER_CHUNK * sizeof(uint16_t) + 1];
    size_t recSize = SAMPLES_PER_CHUNK * sizeof(int16_t);

    if (m_state == RECORDING)
    {
        // Read from mic and send to HA in chunks
        if (millis() - m_stateChangeTS >= DEMO_RECORD_SECONDS * 1000)
        {
            stopRecording();
            return;
        }
        mic_buf[0] = m_stt_binary_handler_id; // first byte is the handler id
        size_t got;
        if (!instance.mic.read((int16_t *)(mic_buf + 1), recSize, &got))
        {
            Serial.println(F("[VP] I2S read error"));
            return;
        }

        if (got == 0)
        {
            return;
        }
        // Serial.print(F("[VP] Sending mic chunk, samples: "));
        // Serial.println(got);
        m_ws.sendBIN((uint8_t *)mic_buf, got + 1);
        yield(); // yield WiFi
        return;
    }

    if (m_state == PLAYING)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Fetching TTS MP3...");

            // Stream MP3 from Home Assistant
            if (m_audio.connecttohost(m_ttsUrl.c_str()))
            {
                m_ttsUrl = "";
                Serial.println("Playing audio...");
                while (m_audio.isRunning())
                {
                    m_audio.loop(); // decode and send PCM to I2S
                }
                Serial.println("Playback finished");
            }
            else
            {
                Serial.println("Failed to connect to TTS URL");
            }
        }

    }
}
