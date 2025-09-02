#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "ConfigManager.h"  
#include "PompCommand.h"
#include <ArduinoJson.h>
 

#define RESET_BUTTON_PIN D2 //D2(gpio4)d:\Personel\Project\plantcareApp\NodeMCU_IoT\sketch_aug1a\PompCommand.h

#define RESET_LED_PING D1 // D1(gpio5)
#define RESET_LED_PINR D4  // ya da #define LED_PIN 2


#define WIFI_LED_PING D8 
#define WIFI_LED_PINR D7


#define Pomp_1 D5 

#define fakeLed D6 


unsigned long buttonPressStart = 0;
unsigned long connectWifi=0;
const unsigned long resetHoldTime = 5000;  // 5 second
bool   relayOn            = false;
unsigned long relayOffAt  = 0; // millis zaman damgası
const bool RELAY_ACTIVE_LOW = true; // Röleniz aktif HIGH ise false yapın

ESP8266WebServer server(80);

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient); 
 
 
void handleConfigSave() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"body yok\"}");
    return;
  }
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"json hatası\"}");
    return;
  }

  config.wifi_ssid = doc["wifi_ssid"].as<String>();
  config.wifi_password = doc["wifi_password"].as<String>();
  config.mqtt_server = doc["mqtt_server"].as<String>();
  config.mqtt_port = doc["mqtt_port"];
  config.mqtt_user = doc["mqtt_user"].as<String>();
  config.mqtt_password = doc["mqtt_password"].as<String>();
  config.deviceid = doc["deviceid"].as<String>();

  saveConfig();

  server.send(200, "application/json", "{\"status\":\"ok\",\"restarting\":true}");
  delay(2000);
  ESP.restart(); 

}

void startWebConfig() {
  IPAddress apIP(192, 168, 4, 1);        // Cihazın IP adresi
  IPAddress gateway(192, 168, 4, 1);     // Genellikle aynı IP
  IPAddress subnet(255, 255, 255, 0);      // Alt ağ maskesi


  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, gateway, subnet);
    WiFi.softAP("smartVase", "12345678");
  Serial.println("Config AP opening.");
 // server.serveStatic("/", SPIFFS, "/index.html");
  server.on("/save", HTTP_POST, handleConfigSave);
  server.begin();
  Serial.print("WiFi roter");
  Serial.println(WiFi.softAPIP());
}

void connectWiFi() {
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
  Serial.print("WiFi connecting");

   digitalWrite(WIFI_LED_PING,  LOW);
   digitalWrite(WIFI_LED_PINR,  HIGH);

  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    if (millis() - timeout > 15000) break;
  }

  Serial.println("");
  Serial.println("WiFi bağlı!");
  Serial.print("IP adresi: ");

  digitalWrite(WIFI_LED_PING, HIGH);
  digitalWrite(WIFI_LED_PINR,  LOW);
  connectWifi=1;

  Serial.println(WiFi.localIP());  
}

 
void setRelay(bool on) {
  relayOn = on;
 
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(Pomp_1, on ? LOW : HIGH);
  } else {
    digitalWrite(Pomp_1, on ? HIGH : LOW);
  }
 
  
}

// Aç ve belirli süre sonra otomatik kapat
void startRelayFor(unsigned long ms) {
  setRelay(true);
  relayOffAt = millis() + ms;
}
void connectMQTT() {


  mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  mqttClient.setCallback(callback); 
  while (!mqttClient.connected()) {
 
    Serial.print("MQTT broker connecting...");
    Serial.print(" Device Id ");
    Serial.print(config.deviceid.c_str());
 

    if (mqttClient.connect(config.deviceid.c_str(), config.mqtt_user.c_str(), config.mqtt_password.c_str())) {
      Serial.println("MQTT connected");
      String commandSubscribe = String(config.deviceid)  + "/command";
 
      mqttClient.subscribe(commandSubscribe.c_str());
    } else {
      Serial.print("cannot connect: ");
      Serial.print(mqttClient.state());

      Serial.print("  IP adresi: ");
      Serial.println(WiFi.localIP());  

      delay(5000);
    }
  }
}
 


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mesaj geldi [");
  Serial.print(topic);
  Serial.print("] ");
 
  unsigned long relayStartTime = 0;
  unsigned long relayDuration = 0;
  bool relayActive = false;

  String jsonStr;

  // Payload'u stringe çevir
  for (int i = 0; i < length; i++) {
    jsonStr += (char)payload[i];
  }
  Serial.println(jsonStr);

  StaticJsonDocument<1024> doc; // Bellek boyutu ayarı
  DeserializationError error = deserializeJson(doc, jsonStr);

  if (error) {
    Serial.print("JSON parse hatası: ");
    Serial.println(error.f_str());
    return;
  }

  const char* command = doc["command"];
 

  if (String(command) == "water") {

    int value = doc["value"];
    int time = doc["time"];  
    Serial.printf("Komut: %s, Değer: %d, Süre: %d\n", command, value, time);
    
    if (value == 1)
    {
        Serial.println("Röle çalıştırılıyor...");
        setRelay(true);

        relayActive = true;
        relayStartTime = millis();
        relayDuration = (unsigned long)time * 1000; // ms’ye çevir
        delay(relayDuration);
      setRelay(false);
       Serial.println("Röle Durduruluyor...");
    }else
    {
        Serial.println("Röle Durduruluyor...");
        setRelay(false);
    }  
    // digitalWrite(RELAY_PIN, HIGH);
  }

  if (String(command) == "SaveSetting") {
      savePompConfig(doc);
  } 

   
 
  
}

void setupOTA() {
  ArduinoOTA.setHostname(config.deviceid.c_str());
  ArduinoOTA.begin();
  Serial.println("OTA ready.");
}

void setup() {

  Serial.begin(115200); 
  
  pinMode(fakeLed, OUTPUT);//silinecek
  digitalWrite(fakeLed,  HIGH); //silinecek

  pinMode(Pomp_1, OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT); 


  digitalWrite(Pomp_1,  LOW);  // Aktif düşük


  pinMode(RESET_LED_PING, OUTPUT);
  pinMode(RESET_LED_PINR, OUTPUT);

  pinMode(WIFI_LED_PING, OUTPUT);
  pinMode(WIFI_LED_PINR, OUTPUT);

  digitalWrite(WIFI_LED_PING,  LOW);
  digitalWrite(WIFI_LED_PINR,  LOW);

  digitalWrite(RESET_LED_PING,  LOW);
  digitalWrite(RESET_LED_PINR,  LOW);  // Aktif düşük

 

  if (!loadConfig()) {
    startWebConfig();
    return;
  }else {
    connectWiFi();

    secureClient.setInsecure();

    if (WiFi.status() != WL_CONNECTED) {
      startWebConfig();
      return;
    }

      server.on("/health", HTTP_GET, []() {
      server.send(200, "application/json", "{\"status\":\"alive\"}");
    });
    server.begin();
 
    connectMQTT();
    setupOTA();
  } 
}

void loop() {
 

   //Serial.println("loop ");
    ArduinoOTA.handle();

    server.handleClient();

    if (connectWifi==1)
    {
      if (!mqttClient.connected()) {
          connectMQTT();
      }
      mqttClient.loop();
    }

    /*
    static unsigned long lastPublish = 0;

    //Press reset button
  if (digitalRead(RESET_BUTTON_PIN) == HIGH) {

        Serial.println("Pressed ");
          
        if (buttonPressStart == 0) {
          buttonPressStart = millis();
        }

        digitalWrite(RESET_LED_PING, millis() % 500 < 250 ? LOW : HIGH);
        digitalWrite(RESET_LED_PINR, millis() % 500 < 250 ? HIGH : LOW);  // Aktif düşük

        if (millis() - buttonPressStart >= resetHoldTime) {

          SPIFFS.begin();
          SPIFFS.remove("/config.json");
          SPIFFS.remove("/deviceSetting.json");          
          SPIFFS.end();
          Serial.println("The button was held for 5 seconds! Settings are resetting....");
          digitalWrite(RESET_LED_PING,  LOW);
          digitalWrite(RESET_LED_PINR,  LOW);  // Aktif düşük

          delay(1000);
          ESP.restart();

        }

    } else {
        // If the button is released, the counter is reset.
        buttonPressStart = 0;
    } 
    */
   // loadPompConfig();

     
}
