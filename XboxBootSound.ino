#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h" // *** CHANGED from AudioGeneratorMP3 ***
#include "AudioOutputI2S.h"    // Yes, I2S, but we'll set it to DAC mode

// --- Define your sound file ---
const char* BOOT_SOUND_FILE = "/bootsound.wav"; // *** CHANGED to .wav ***

// --- Define Pins ---
// (Sense pin is removed for automatic playback)
// GPIO 25 is used by the DAC, so we don't need a define

// --- Audio Objects ---
AudioGeneratorWAV *wav; // *** CHANGED to wav object ***
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out; // This object can also control the internal DAC

// State variable
// (Removed for automatic playback)
// bool xboxIsOn = false;

void setup() {
  Serial.begin(115200);
  Serial.println("\nInitializing Internal DAC Boot Sound Player (WAV)...");

  // Set the sense pin as an input
  // pinMode(XBOX_SENSE_PIN, INPUT); // (Removed)

  // --- Initialize SPIFFS ---
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted.");

  // Check if the sound file exists
  if (!SPIFFS.exists(BOOT_SOUND_FILE)) {
    Serial.print("ERROR: Boot sound file not found: ");
    Serial.println(BOOT_SOUND_FILE);
    Serial.println("Please upload the WAV file using 'ESP32 Sketch Data Upload'");
  } else {
    Serial.println("Boot sound file found.");
  }

  // --- Initialize Audio Objects ---
  // We are not creating them yet, just setting pointers to null
  wav = nullptr; // *** CHANGED ***
  file = nullptr;
  
  // This is the magic line:
  // new AudioOutputI2S(0, 1) means: (port 0, mode 1)
  // Mode 1 = Internal DAC. Mode 0 = External I2S.
  out = new AudioOutputI2S(0, 1); 
  out->SetGain(1.0); // Set volume (0.0 to 4.0)

  // Check initial power state
  // (Removed - we will just play the sound immediately)

  // --- Start playing sound immediately ---
  Serial.println("Playing boot sound automatically...");
  file = new AudioFileSourceSPIFFS(BOOT_SOUND_FILE);
  wav = new AudioGeneratorWAV(); // *** CHANGED ***
  if (!wav->begin(file, out)) { // *** CHANGED ***
    Serial.println("Error starting WAV decoder!");
    delete wav; wav = nullptr; // *** CHANGED ***
    delete file; file = nullptr;
  }
}

void loop() {
  // The main loop *must* call wav->loop() to keep the sound playing
  if (wav && wav->isRunning()) { // *** CHANGED ***
    if (!wav->loop()) { // *** CHANGED ***
      // Sound finished playing
      wav->stop(); // *** CHANGED ***
      delete wav; wav = nullptr; // *** CHANGED ***
      delete file; file = nullptr;
      Serial.println("Sound finished.");
    }
  }

  // A small delay
  delay(50); // A bit shorter delay for smoother audio processing
}
