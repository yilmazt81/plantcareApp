#include <FS.h>
#include <ArduinoJson.h>

struct Config {
  String wifi_ssid;
  String wifi_password;
  String mqtt_server;
  int mqtt_port;
  String mqtt_user;
  String mqtt_password;
  String deviceid; 
};

Config config;

bool loadConfig() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS başlatılamadı.");
    return false;
  }
  if (!SPIFFS.exists("/config.json")) {
    //Serial.println("Config dosyası bulunamadı.");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Config dosyası açılamadı.");
    return false;
  }

  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("JSON parse hatası.");
    return false;
  }

  config.wifi_ssid = doc["wifi_ssid"].as<String>();
  config.wifi_password = doc["wifi_password"].as<String>();
  config.mqtt_server = doc["mqtt_server"].as<String>();
  config.mqtt_port = doc["mqtt_port"] | 1883;
  config.mqtt_user = doc["mqtt_user"].as<String>();
  config.mqtt_password = doc["mqtt_password"].as<String>();
  config.deviceid = doc["deviceid"].as<String>();

  Serial.println("Config yüklendi.");
  return true;
}

bool saveConfig() {
  StaticJsonDocument<1024> doc;
  doc["wifi_ssid"] = config.wifi_ssid;
  doc["wifi_password"] = config.wifi_password;
  doc["mqtt_server"] = config.mqtt_server;
  doc["mqtt_port"] = config.mqtt_port;
  doc["mqtt_user"] = config.mqtt_user;
  doc["mqtt_password"] = config.mqtt_password;
  doc["deviceid"] = config.deviceid;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Config dosyası yazılamadı.");
    return false;
  }

  serializeJson(doc, configFile);
  configFile.close();
  Serial.println("Config kaydedildi.");
  return true;
}
