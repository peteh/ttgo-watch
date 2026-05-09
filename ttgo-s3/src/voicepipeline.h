#pragma once
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <Audio.h> // Audio output
// TODO extract all these infos
#include "config.h" // WiFi, HA host, token, pipeline

class VoicePipeline
{
public:
    enum PipelineState
    {
        IDLE,
        STARTING,
        RECORDING,
        STOPPING,
        PLAYING
    };

    VoicePipeline() = default;
    ~VoicePipeline() = default;

    void init();

    void loop();

    void startRecording();
    void stopRecording();

    long getLastStateChangeTS() const { return m_stateChangeTS; }
    PipelineState getState() const { return m_state; }

private:
    Audio m_audio = Audio(false, 3, I2S_NUM_1);
    String m_ttsUrl = "";

    PipelineState m_state = IDLE;
    long m_stateChangeTS = 0;

    bool m_authed = false;
    uint8_t m_runId = 0;

    void onWSEvent(WStype_t type, uint8_t *payload, size_t length);

    WebSocketsClient m_ws;

    uint8_t m_stt_binary_handler_id = 0;

    void transitionToState(PipelineState newState);

    const int SAMPLE_RATE = 16000;
    const int CH_BITS = 16; // bits per sample

    // Chunking
    const size_t SAMPLES_PER_CHUNK = 512; // 100 ms at 16kHz

    // Record length for demo (seconds)
    const uint32_t DEMO_RECORD_SECONDS = 4;

public:
};