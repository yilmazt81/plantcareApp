#pragma once
#include <LittleFS.h>
#include <ArduinoJson.h>

struct Config {
  String wifi_ssid;
  String wifi_password;
  String mqtt_server;
  int    mqtt_port;
  String mqtt_user;
  String mqtt_password;
  String device_id;
};

Config config;

// ======================
// Config Dosyasını Yükle
// ======================
bool loadConfig() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS başlatılamadı.");
    return false;
  }
  if (!LittleFS.exists("/config.json")) {
    Serial.println("Config dosyası bulunamadı.");
    return false;
  }

  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Config dosyası açılamadı.");
    return false;
  }

  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size + 1]);
  configFile.readBytes(buf.get(), size);
  buf[size] = '\0';
  configFile.close();

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.print("JSON parse hatası: ");
    Serial.println(error.c_str());
    return false;
  }

  config.wifi_ssid     = doc["wifi_ssid"]     | "";
  config.wifi_password = doc["wifi_password"] | "";
  config.mqtt_server   = doc["mqtt_server"]   | "";
  config.mqtt_port     = doc["mqtt_port"]   ;
  config.mqtt_user     = doc["mqtt_user"]     | "";
  config.mqtt_password = doc["mqtt_password"] | "";
  config.device_id      = doc["device_id"]      | "";

  Serial.println("Config yüklendi (LittleFS).");
  return true;
}

// ======================
// Config Dosyasını Kaydet
// ======================
bool saveConfig() {
  StaticJsonDocument<1024> doc;
  doc["wifi_ssid"]     = config.wifi_ssid;
  doc["wifi_password"] = config.wifi_password;
  doc["mqtt_server"]   = config.mqtt_server;
  doc["mqtt_port"]     = config.mqtt_port;
  doc["mqtt_user"]     = config.mqtt_user;
  doc["mqtt_password"] = config.mqtt_password;
  doc["device_id"]      = config.device_id;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Config dosyası yazılamadı.");
    return false;
  }

  serializeJson(doc, configFile);
  configFile.close();
  Serial.println("Config kaydedildi (LittleFS).");
  return true;
}

// ======================
// Config Dosyasını Sıfırla
// ======================
bool resetConfig() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS başlatılamadı.");
    return false;
  }
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
    Serial.println("Config sıfırlandı (LittleFS).");
    return true;
  }
  Serial.println("Config zaten yok (LittleFS).");
  return false;
}
