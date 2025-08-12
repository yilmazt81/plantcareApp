#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>   // ESP8266 için BearSSL
// #include "ConfigManager.h"   // LittleFS uyumlu sürüm


// =================== Donanım ===================
#define SOIL_SENSOR_PIN   A0
#define RESET_BUTTON_PIN  D3
const unsigned long resetHoldTime = 5000;  // 5 saniye
unsigned long buttonPressStart = 0;

// =================== Ağ / Servisler ===================
ESP8266WebServer server(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;

// TLS/MQTT
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);
BearSSL::X509List* g_caCert = nullptr;   // CA zinciri burada tutulur

// =================== Config Yapısı ===================
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

const char* MQTT_HOST = "m6e105d6.ala.eu-central-1.emqxsl.com";
const uint16_t MQTT_PORT = 8883;
const char* MQTT_USER = "waterUserName";
const char* MQTT_PASS = "usr_26f924f3d92";
const char* MQTT_CLIENT_ID = "hP3t8DuEs2"; // device_id

// =================== İleri Bildirim/Debug ===================
String wlStatusToString(wl_status_t s) {
  switch (s) {
    case WL_IDLE_STATUS:      return "IDLE";
    case WL_NO_SSID_AVAIL:    return "NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:   return "SCAN_COMPLETED";
    case WL_CONNECTED:        return "CONNECTED";
    case WL_CONNECT_FAILED:   return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:  return "CONNECTION_LOST";
    case WL_DISCONNECTED:     return "DISCONNECTED";
    default: return "UNKNOWN(" + String((int)s) + ")";
  }
}

// =================== LittleFS: Config Yükle/Kaydet/Sıfırla ===================
bool loadConfig() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS başlatılamadı.");
    return false;
  }
  if (!LittleFS.exists("/config.json")) {
    Serial.println("Config dosyası bulunamadı.");
    return false;
  }

  File f = LittleFS.open("/config.json", "r");
  if (!f) {
    Serial.println("Config dosyası açılamadı.");
    return false;
  }
  size_t size = f.size();
  std::unique_ptr<char[]> buf(new char[size + 1]);
  f.readBytes(buf.get(), size);
  buf[size] = '\0';
  f.close();

  StaticJsonDocument<1024> doc;
  auto err = deserializeJson(doc, buf.get());
  if (err) {
    Serial.print("JSON parse hatası: ");
    Serial.println(err.c_str());
    return false;
  }

  config.wifi_ssid     = doc["wifi_ssid"]     | "";
  config.wifi_password = doc["wifi_password"] | "";


  config.mqtt_server   = MQTT_HOST;
  config.mqtt_port     = MQTT_PORT;
  config.mqtt_user     = MQTT_USER;        // düzeltilmiş
  config.mqtt_password = MQTT_PASS;
  config.device_id     = MQTT_CLIENT_ID;

  Serial.println("Config yüklendi (LittleFS).");
  return true;
}

bool saveConfig() {
  StaticJsonDocument<1024> doc;
  doc["wifi_ssid"]     = config.wifi_ssid;
  doc["wifi_password"] = config.wifi_password;
  doc["mqtt_server"]   = config.mqtt_server;
  doc["mqtt_port"]     = config.mqtt_port;
  doc["mqtt_user"]     = config.mqtt_user;
  doc["mqtt_password"] = config.mqtt_password;
  doc["device_id"]     = config.device_id;

  File f = LittleFS.open("/config.json", "w");
  if (!f) {
    Serial.println("Config dosyası yazılamadı.");
    return false;
  }
  serializeJson(doc, f);
  f.close();
  Serial.println("Config kaydedildi (LittleFS).");
  return true;
}

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

// =================== Web: /saveConfig ===================
void handleConfigSave() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"body yok\"}");
    return;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"error\":\"json hatası\"}");
    return;
  }

  config.wifi_ssid     = doc["wifi_ssid"].as<String>();
  config.wifi_password = doc["wifi_password"].as<String>();
  // config.mqtt_server   = doc["mqtt_server"].as<String>();
  // config.mqtt_port     = doc["mqtt_port"].as<int>();
  // config.mqtt_user     = doc["mqtt_user"].as<String>();
  // config.mqtt_password = doc["mqtt_password"].as<String>();
  // config.device_id     = doc["device_id"].as<String>();

  saveConfig();
  server.send(200, "application/json", "{\"status\":\"saved\"}");

  delay(1000);
  ESP.restart();
}

// =================== WiFi ===================
void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("smartVase", "12345678");
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP mode: SSID=smartVase, IP=%s\n", ip.toString().c_str());

  // Captive portal DNS
  dnsServer.start(DNS_PORT, "*", ip);
}

void connectWiFi() {
  if (config.wifi_ssid.length() == 0) {
    Serial.println("WiFi config yok, AP moduna geçiliyor.");
    startAP();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());

  Serial.printf("Connecting to WiFi: %s\n", config.wifi_ssid.c_str());
  wl_status_t r = (wl_status_t) WiFi.waitForConnectResult();
  if (r == WL_CONNECTED) {
    Serial.printf("Connected, IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("WiFi connect failed, AP moduna geçiliyor.");
    startAP();
  }
}

// =================== NTP (TLS için zorunlu) ===================
void setupTime() {
  if (WiFi.status() != WL_CONNECTED) return;
  configTime(3 * 3600, 0, "pool.ntp.org", "time.google.com"); // GMT+3
  Serial.print("NTP bekleniyor");
  time_t now = time(nullptr);
  while (now < 1700000000) { // ~2023+
    delay(200);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nSaat ayarlandı.");
}


// =================== TLS / CA ===================
bool setupTLS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount başarısız (TLS).");
    return false;
  }
  File f = LittleFS.open("/emqx_ca.pem", "r");
  if (!f) {
    Serial.println("CA dosyası yok: /emqx_ca.pem");
    return false;
  }
  String pem = f.readString();
  f.close();

  if (g_caCert) { delete g_caCert; g_caCert = nullptr; }
  g_caCert = new BearSSL::X509List(pem.c_str());

  secureClient.setTrustAnchors(g_caCert);
  secureClient.setBufferSizes(1024, 1024);  // MQTT için dengeli
  // secureClient.setInsecure(); // *** SADECE TEŞHİS İÇİN. ÜRETİMDE KULLANMA. ***
  return true;
}

// =================== MQTT ===================
bool connectMQTT() {
  if (mqttClient.connected()) return true;

  Serial.printf("Connecting to MQTT (host=%s, port=%d)...\n",
                config.mqtt_server.c_str(), config.mqtt_port);

  bool ok = mqttClient.connect(
              config.device_id.c_str(),
              config.mqtt_user.c_str(),
              config.mqtt_password.c_str());

  int st = mqttClient.state();
  if (ok) {
    Serial.println("MQTT connected!");
    return true;
  } else {
    Serial.printf("MQTT failed, rc=%d\n", st); // -2 => TCP/TLS connect fail
    return false;
  }
}

void sendSensorData() {
  int soilValue = analogRead(SOIL_SENSOR_PIN);
  String topic = "devices/" + config.device_id + "/soil";
  String payload = "{\"soil\":" + String(soilValue) + "}";

  if (mqttClient.publish(topic.c_str(), payload.c_str())) {
    Serial.println("Data sent: " + payload);
  } else {
    Serial.println("MQTT publish failed");
  }
}

// =================== Setup ===================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Smart Vase starting...");

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
    // devam: AP moduna düşebilmek için yine de ilerleyeceğiz
  } else {
    Serial.println("LittleFS mounted.");
  }

  loadConfig();
  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    setupTime();    // TLS için saat şart
    if (!setupTLS()) {
      Serial.println("TLS/CA kurulamadı! (Teşhis için geçici setInsecure() kullanılabilir)");
    }
  }

  // HTTP statik içerik + API
  server.serveStatic("/", LittleFS, "/index.html");      // UI
  server.serveStatic("/config.json", LittleFS, "/config.json"); // debug
  server.on("/saveConfig", HTTP_POST, handleConfigSave);

  // Android captive portal düzeltmeleri + fallback redirect
  server.onNotFound([]() {
    String hostHeader = server.hostHeader();
    if (hostHeader.indexOf("connectivitycheck.gstatic.com") >= 0 ||
        hostHeader.indexOf("clients3.google.com") >= 0) {
      server.send(204, "text/plain", "");
      return;
    }
    if (server.method() == HTTP_GET) {
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "application/json", "{\"error\":\"not found\"}");
    }
  });

  server.begin();
  Serial.println("HTTP server started");

  // MQTT ayarları
//test edilecek port 
    Serial.println(config.mqtt_port.c_str());
  mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(10);
  mqttClient.setBufferSize(1024);
}

// =================== Loop ===================
void loop() {
  // Captive portal DNS ve web server
  if (WiFi.getMode() & WIFI_AP) {
    dnsServer.processNextRequest();
  }
  server.handleClient();

  // MQTT non-blocking bağlanma & loop
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      static unsigned long lastTry = 0;
      if (millis() - lastTry > 3000) {
        lastTry = millis();
        connectMQTT();
      }
    } else {
      mqttClient.loop();
    }
  }

  // Reset butonu (5 sn)
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart >= resetHoldTime) {
      Serial.println("Reset butonuna uzun basıldı, ayarlar sıfırlanıyor...");
      resetConfig();
      ESP.restart();
    }
  } else {
    buttonPressStart = 0;
  }

  // 5 sn'de bir sensör publish (yalnızca MQTT bağlıyken)
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000 && mqttClient.connected()) {
    lastSend = millis();
    sendSensorData();
  }

  // Periyodik durum logu (non-blocking)
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 10000) {
    lastLog = millis();
    Serial.printf("WiFi:%s(%d)  Mode:%s%s  MQTT:%d\n",
      WiFi.SSID().c_str(), WiFi.status(),
      (WiFi.getMode() & WIFI_STA) ? "STA" : "",
      (WiFi.getMode() & WIFI_AP)  ? "+AP"  : "",
      mqttClient.connected());
  }
}
