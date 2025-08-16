#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "ConfigManager.h"  
#include "PompCommand.h"
#include <ArduinoJson.h>
 

#define RESET_BUTTON_PIN D2 //D2(gpio4)

#define RESET_LED_PING D1 // D1(gpio5)
#define RESET_LED_PINR D4  // ya da #define LED_PIN 2

#define WIFI_LED_PING D8 
#define WIFI_LED_PINR D7


//#define LED_BUILTIN D2
//#define HUMIDITY_BUILTIN D4

#define Pomp_1 D5
#define Pomp_2 D6 


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
  WiFi.softAP("smartVase2", "78945621");
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

 
void setRelay(bool on,int pompNumber) {
  relayOn = on;
  if (pompNumber==1)
  {
    if (RELAY_ACTIVE_LOW) {
      digitalWrite(Pomp_1, on ? LOW : HIGH);
    } else {
      digitalWrite(Pomp_1, on ? HIGH : LOW);
    }
  }else if (pompNumber==2)
  {
    if (RELAY_ACTIVE_LOW) {
      digitalWrite(Pomp_2, on ? LOW : HIGH);
    } else {
      digitalWrite(Pomp_2, on ? HIGH : LOW);
    }
  }
  
}

// Aç ve belirli süre sonra otomatik kapat
void startRelayFor(unsigned long ms,int pompnumber) {
  setRelay(true,pompnumber);
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
    int pomp = doc["pomp"];
    Serial.printf("Komut: %s, Değer: %d, Süre: %d\n Pomp: %d\n", command, value, time,pomp);
    
    if (value == 1)
    {
        Serial.println("Röle çalıştırılıyor...");
        setRelay(true,pomp);

        relayActive = true;
        relayStartTime = millis();
        relayDuration = (unsigned long)time * 1000; // ms’ye çevir
        delay(relayDuration);
        setRelay(false,pomp);
       Serial.println("Röle Durduruluyor...");
    }else
    {
        Serial.println("Röle Durduruluyor...");
        setRelay(false,pomp);
    }  
    // digitalWrite(RELAY_PIN, HIGH);
  }

  if (String(command) == "SaveSetting") {
      savePompConfig(doc);
  } 
/*
  String jsonStr;
  jsonStr.reserve(length + 1);

  for (unsigned int i = 0; i < length; i++) 
  
  jsonStr += (char)payload[i];

  // JSON parse
  StaticJsonDocument<500> doc;  // gelen veri küçük, 256 yeterli

  // Beklenen alanlar
  const char* command = doc["command"] | "";
  int value           = doc["value"]   | 0;
  unsigned long timeS = doc["time"]    | 0; // saniye
  int pompNumber = doc["pomp"] | 1;
  // Komut işleme
  if (strcmp(command, "water") == 0) {
    if (value == 1) {
      unsigned long durationMs = timeS * 1000UL;
      if (durationMs == 0) {
        // Süre yoksa açık bırakma (manuel kapatma beklenir)
        setRelay(true,pompNumber);
      } else {
        startRelayFor(durationMs,pompNumber);
      }
      Serial.println(" Pompa start: ");
       Serial.print(pompNumber);
      // Geri bildirim
      //StaticJsonDocument<192> out;
      //out["event"] = "relay_on";
      //out["remaining_sec"] = timeS;
      //char buf[192];
      //serializeJson(out, buf);
      //mqttClient.publish(PUB_TOPIC.c_str(), buf, true);

    } else { // value == 0
      setRelay(false,pompNumber);
      //relayOffAt = 0;
      Serial.println(" Pompa Stop: ");
      Serial.print(pompNumber);
      //StaticJsonDocument<128> out;
      //out["event"] = "relay_off";
      //char buf[128];
      //serializeJson(out, buf);
      //mqttClient.publish(PUB_TOPIC.c_str(), buf, true);
    }
  }

  */
}

void setupOTA() {
  ArduinoOTA.setHostname(config.deviceid.c_str());
  ArduinoOTA.begin();
  Serial.println("OTA ready.");
}

void setup() {

  Serial.begin(115200); 
  
  
  pinMode(Pomp_2, OUTPUT);
  pinMode(Pomp_1, OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT); 

  digitalWrite(Pomp_2,  LOW);
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

      digitalWrite(Pomp_1,  HIGH);
     delay(1000);
      digitalWrite(Pomp_2,  HIGH);
       delay(1000);
  digitalWrite(Pomp_2,  LOW);

      delay(1000);
  digitalWrite(Pomp_1,  LOW);  // Aktif düşük
    delay(1000);
    static unsigned long lastPublish = 0;

    //Press reset button
  /** if (digitalRead(RESET_BUTTON_PIN) == HIGH) {

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
    
    loadPompConfig();

    
 

}
