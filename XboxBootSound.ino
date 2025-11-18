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

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <FS.h>

#include <AudioFileSourceSPIFFS.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

// WiFi AP
const char* ssid = "XBOX_SOUND_MOD";
const char* password = "12345678";
IPAddress apIP(192, 168, 4, 1);

// Web server
WebServer server(80);

// Audio objects
AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSPIFFS *file = nullptr;
AudioOutputI2S *out = nullptr;

bool audioPlaying = false;

// ----------------------------
// Minimal web page (separate)
// ----------------------------
const char* PAGE = 
"<!DOCTYPE html>"
"<html>"
"<head><title>Xbox Sound</title></head>"
"<body style=\"font-family:Arial;background:#111;color:#eee;padding:20px\">"
"<h2>Xbox 360 Sound Manager</h2>"

"<h3>Files:</h3>"
"<div id='list'>Loading...</div>"

"<h3>Upload WAV</h3>"
"<form method='POST' action='/upload' enctype='multipart/form-data'>"
"<input type='file' name='data' accept='.wav'>"
"<input type='submit' value='Upload'>"
"</form>"

"<script>"
"function loadList() {"
"  fetch('/list').then(function(r){ return r.json(); }).then(function(files){"
"    var html='';"
"    for (var i=0;i<files.length;i++) {"
"      var f = files[i];"
"      html += f + "
"        \" <button onclick=\\\"fetch('/play?f=\" + f + \"')\\\">Play</button>\" + "
"        \" <button onclick=\\\"fetch('/delete?f=\" + f + \"').then(function(){ loadList(); })\\\">Delete</button><br>\";"
"    }"
"    document.getElementById('list').innerHTML = html ? html : 'No files';"
"  });"
"}"
"loadList();"
"</script>"

"</body>"
"</html>";

// ----------------------------
// Audio control
// ----------------------------

void stopAudio() {
  if (wav) {
    if (wav->isRunning()) wav->stop();
    delete wav;
    wav = nullptr;
  }
  if (file) { delete file; file = nullptr; }
  audioPlaying = false;
}

void startAudio(const char *fname) {
  stopAudio();

  if (!SPIFFS.exists(fname)) {
    Serial.printf("File missing: %s\n", fname);
    return;
  }

  file = new AudioFileSourceSPIFFS(fname);
  wav = new AudioGeneratorWAV();

  Serial.printf("Start WAV: %s\n", fname);

  if (!wav->begin(file, out)) {
    Serial.println("WAV decoder failed");
    stopAudio();
    return;
  }

  audioPlaying = true;
}

void audioLoop() {
  if (wav && wav->isRunning()) {
    if (!wav->loop()) {
      stopAudio();
    }
  }
}

// ----------------------------
// Web handlers
// ----------------------------

void handleRoot() {
  server.send(200, "text/html", PAGE);
}

void handleList() {
  String json = "[";
  File root = SPIFFS.open("/");
  File f = root.openNextFile();
  bool first = true;

  while (f) {
    String name = f.name();
    if (name.endsWith(".wav")) {
      if (!first) json += ",";
      first = false;
      json += "\"" + name.substring(1) + "\"";
    }
    f = root.openNextFile();
  }

  json += "]";
  server.send(200, "application/json", json);
}

void handlePlay() {
  if (!server.hasArg("f")) {
    server.send(400, "text/plain", "Missing f");
    return;
  }
  String file = "/" + server.arg("f");
  startAudio(file.c_str());
  server.send(200, "text/plain", "playing");
}

void handleDelete() {
  if (!server.hasArg("f")) {
    server.send(400, "text/plain", "Missing f");
    return;
  }
  String file = "/" + server.arg("f");
  stopAudio();
  SPIFFS.remove(file);
  server.send(200, "text/plain", "deleted");
}

// File upload
File uploadFile;

void handleUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    if (!filename.endsWith(".wav")) return;

    uploadFile = SPIFFS.open(filename, "w");

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);

  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) uploadFile.close();
  }
}

// ----------------------------
// Setup
// ----------------------------

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);

  // I2S in DAC mode (GPIO25)
  out = new AudioOutputI2S(0, 1);   // internal DAC
  out->SetOutputModeMono(true);     // mono = more stable
  out->SetGain(1.0);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));

  server.on("/", handleRoot);
  server.on("/list", handleList);
  server.on("/play", handlePlay);
  server.on("/delete", handleDelete);
  server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleUpload);

  server.begin();
  Serial.println("Ready on http://192.168.4.1/");
}

// ----------------------------
// Loop
// ----------------------------

void loop() {
  server.handleClient();
  audioLoop();
}

