/*
 * Xbox 360 Sound Manager & Player - AP Mode
 * * This sketch creates an Access Point (AP) and hosts a web server
 * allowing file uploads (.wav), file listing, and playback control.
 * It uses the ESP32's internal DAC (GPIO 25) for sound output.
 * * * HARDWARE NOTES:
 * - Speaker must be connected to GPIO 25 and GND.
 * - This version omits the sense pin logic for simplicity during testing.
 *
 * * WEB ACCESS:
 * - SSID: "XBOX_SOUND_MOD"
 * - Password: "12345678"
 * - Access Web Interface at: http://192.168.4.1/
 */

// --- Standard ESP32 Libraries ---
#include "WiFi.h"
#include "WebServer.h"
#include "FS.h"
#include "SPIFFS.h"

// --- Audio Libraries (Requires ESP8266Audio library) ---
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// --- Network Configuration ---
const char *ssid = "XBOX_SOUND_MOD";
const char *password = "12345678";
IPAddress apIP(192, 168, 4, 1);

// --- Web Server Setup ---
WebServer server(80);

// --- Audio Objects ---
AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSPIFFS *file = nullptr;
AudioOutputI2S *out = nullptr; // Used for DAC output

// --- Global Audio State ---
bool audioIsPlaying = false;
unsigned long playbackStartTime = 0;

// =========================================================
//              AUDIO DIAGNOSTIC CALLBACKS
// =========================================================

// Called when audio properties are discovered (helpful for debugging format issues)
void audio_info(const char *name, int value) {
  Serial.printf("INFO: %s = %d\n", name, value);
}

// Called when general information is available
void audio_info(const char *info) {
  Serial.print("INFO: ");
  Serial.println(info);
}

// Called when an error occurs during decoding or playback
void audio_error(const char *info) {
  Serial.print("AUDIO ERROR: ");
  Serial.println(info);
}

// Called when WAV file reaches the end
void audio_eof_wav(const char *info) {
  Serial.println("INFO: WAV file finished (End Of File)");
}

// (Other callbacks required by the library, but not used for WAV)
void audio_id3data(const char *info) {}
void audio_showstation(const char *info) {}
void audio_showstreamtitle(const char *info) {}


// =========================================================
//                  AUDIO FUNCTIONS
// =========================================================

// Function to stop audio playback and clean up memory
void stopAudio() {
  if (wav && wav->isRunning()) {
    wav->stop();
    Serial.println("Audio stopped.");
  }
  if (wav) {
    delete wav;
    wav = nullptr;
  }
  if (file) {
    delete file;
    file = nullptr;
  }
  audioIsPlaying = false;
}

// Function to start playing a specific file
void startAudio(const char *filename) {
  stopAudio(); // Stop anything currently playing
  
  if (!SPIFFS.exists(filename)) {
    Serial.printf("Error: File not found: %s\n", filename);
    return;
  }

  // Initialize and start playback
  file = new AudioFileSourceSPIFFS(filename);
  wav = new AudioGeneratorWAV();
  
  if (wav->begin(file, out)) {
    Serial.printf("Playing WAV file: %s\n", filename);
    audioIsPlaying = true;
    playbackStartTime = millis();
  } else {
    Serial.println("Error starting WAV decoder!");
    stopAudio();
  }
}

// Function to handle the continuous audio loop
void audioLoop() {
  if (wav && wav->isRunning()) {
    if (!wav->loop()) {
      // Sound finished playing
      Serial.println("Sound finished playing.");
      stopAudio();
    }
  }
