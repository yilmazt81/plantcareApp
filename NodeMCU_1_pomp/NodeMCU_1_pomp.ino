 
 
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "ConfigManager.h"  
#include "PompCommand.h"
#include <ArduinoJson.h>  
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//#include <HttpClient.h>
   


#define RESET_BUTTON_PIN D2 //D2(gpio4)
 

#define WIFI_LED_PING  D4 //Buna bak yanlis D4 olabilir
#define WIFI_LED_PINR  D3 

 
 
#define Pomp_PIN D1
  

unsigned long buttonPressStart = 0;
unsigned long connectWifi=0;
const unsigned long resetHoldTime = 5000;  // 5 second
bool   relayOn            = false;
unsigned long relayOffAt  = 0; // millis zaman damgası
long lastRunMinute=0;
 

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
  WiFi.softAP("smartVase3", "85236974");
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

  
void  OpenPomp() {
 
   
  digitalWrite(Pomp_PIN,HIGH);
     
  String  pub_topic=String(config.deviceid) + "/status";
  String message="pompstatus|"+String(1)+"|1"; 

  PublishMessage(pub_topic,message);
} 

void  ClosePomp() {
 
   
  digitalWrite(Pomp_PIN,LOW);
     
  String  pub_topic=String(config.deviceid) + "/status";
  String message="pompstatus|"+String(1)+"|0"; 

  PublishMessage(pub_topic,message);
 
} 

void PublishMessage(String pub_topic,String message){
   mqttClient.publish(pub_topic.c_str(),message.c_str(), false);
  mqttClient.loop();
}
 
 
void connectMQTT() {


  mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  mqttClient.setCallback(callback); 
 

  while (!mqttClient.connected()) {
 
    Serial.print("MQTT broker connecting...");
    Serial.print(" Device Id ");
    Serial.print(config.deviceid.c_str());
 
    mqttClient.setKeepAlive(30);        // varsayılan 15 – 30 daha güvenli
    mqttClient.setSocketTimeout(15);    // ağ yavaşsa artır
    mqttClient.setBufferSize(1024);  
    if (mqttClient.connect(config.deviceid.c_str(), config.mqtt_user.c_str(), config.mqtt_password.c_str())) {
      Serial.println("MQTT connected");
      String commandSubscribe = String(config.deviceid)  + "/command"; 
      mqttClient.subscribe(commandSubscribe.c_str());
    } else {
      Serial.print("cannot connect: ");
      digitalWrite(WIFI_LED_PING,  HIGH);
      digitalWrite(WIFI_LED_PINR,  LOW);
      Serial.print(mqttClient.state());
      delay(2000); 
      Serial.print("  IP adresi: ");
      Serial.println(WiFi.localIP());  

      delay(5000);
    }
  }
}
 



void setupOTA() {
  ArduinoOTA.setHostname(config.deviceid.c_str());
 
  ArduinoOTA.begin();
  Serial.println("OTA ready.");
}


 


void setup() {

  Serial.begin(115200);   
  
  pinMode(Pomp_PIN, OUTPUT);   
  pinMode(RESET_BUTTON_PIN, INPUT); 
  digitalWrite(Pomp_PIN,  LOW); 

  pinMode(WIFI_LED_PING, OUTPUT);
  pinMode(WIFI_LED_PINR, OUTPUT);

  digitalWrite(WIFI_LED_PING,  LOW);
  digitalWrite(WIFI_LED_PINR,  LOW);

 

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

    setupTime();
  } 
}
  
 

 
void loop() {
 
 
    ArduinoOTA.handle();

    server.handleClient();
 
 
    if (connectWifi==1)
    {
      
      if (!mqttClient.connected()) {
          connectMQTT();
      }
      mqttClient.loop();
    }

       
    static unsigned long lastPublish = 0;
 
  //Press reset button
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {

        Serial.println("Pressed ");
        
        if (buttonPressStart == 0) {
          buttonPressStart = millis();
        }

        digitalWrite(WIFI_LED_PING, millis() % 500 < 250 ? LOW : HIGH);
        digitalWrite(WIFI_LED_PINR, millis() % 500 < 250 ? HIGH : LOW);  // Aktif düşük

        if (millis() - buttonPressStart >= resetHoldTime) {

          SPIFFS.begin();
          SPIFFS.remove("/config.json");
          SPIFFS.remove("/deviceSetting.json");          
          SPIFFS.end();
          Serial.println("The button was held for 5 seconds! Settings are resetting....");
    

          digitalWrite(WIFI_LED_PING,  LOW);        
          digitalWrite(WIFI_LED_PING,  LOW);
        
          delay(1000);
          ESP.restart();

        }
      
    } else {
        // If the button is released, the counter is reset.
        buttonPressStart = 0;
    }
  
 
     
    if (loadPompConfig()){
      //  bool r =Getforecast();
      CheckDateTimeForWork();
    }  
    
    delay(2000);

}

void setupTime() {
  if (WiFi.status() != WL_CONNECTED) 
    return;
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

int* parseDaysList(const String& s, int& count) {
 static int result[7];  // Haftada en fazla 7 gün olur
  count = 0;
  int start = 0;

  while (start < s.length() && count < 7) {
    int commaIndex = s.indexOf(',', start);
    if (commaIndex == -1) break;

    String numStr = s.substring(start, commaIndex);
    numStr.trim();
    if (numStr.length() > 0) {
      result[count++] = numStr.toInt();
    }
    start = commaIndex + 1;
  }

  return result;  // pointer döner
}

 


 


 

bool contains(int arr[], int size, int value) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == value) {
      return true;  // bulundu
    }
  }
  return false; // yok
}


void callback(char* topic, byte* payload, unsigned int length) {

 
  Serial.print("Mesaj geldi [");
  Serial.print(topic);
  Serial.print("] ");

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  

  int minute = timeinfo->tm_min;
 
  unsigned long relayStartTime = 0;
  unsigned long relayDuration = 0;
  bool relayActive = false;

  String jsonStr;

  // Payload'u stringe çevir
  for (int i = 0; i < length; i++) {
    jsonStr += (char)payload[i];
  }
  Serial.println(jsonStr);
  if (jsonStr=="")
     return;
  StaticJsonDocument<1024> doc; // Bellek boyutu ayarı
  DeserializationError error = deserializeJson(doc, jsonStr);

  if (error) {
    Serial.print("JSON parse hatası callback : ");
    Serial.println(error.f_str());
    Serial.println(error.f_str());
    return;
  }

  const char* command = doc["command"];
  
  if (String(command) == "water") {

    int value = doc["value"];
    int time = doc["time"];
    int pomp = doc["pomp"];
    if (value==1)
        Serial.println("Röle çalıştırılıyor...");    
          lastRunMinute= minute;    
        OpenPomp();       
        relayActive = true;
        relayStartTime = millis();
        relayDuration = (unsigned long)time * 1000; // ms’ye çevir
        delay(relayDuration); 
        ClosePomp();
        Serial.println("Röle Durduruluyor...");
    }else
    {
         if (lastRunMinute != minute) 
          lastRunMinute = -1;
       // Serial.println("Röle Durduruluyor...");
         ClosePomp();
    }  
    // digitalWrite(RELAY_PIN, HIGH);
  

  if (String(command) == "SaveSetting") {
      savePompConfig(doc);
  } 
 
}


void CheckDateTimeForWork() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  
  unsigned long relayStartTime = 0;
  unsigned long relayDuration = 0;
  bool relayActive = false;

  if (!timeinfo) {
    Serial.println("Zaman bilgisi alinamadi");
    return;
  }

  const char* gunler[] = { "Pazar","Pazartesi","Salı","Çarşamba","Perşembe","Cuma","Cumartesi" };
  int hour   = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  int day    = timeinfo->tm_wday; // 0=Pazar ... 6=Cumartesi
 
  if (pompconfig.pompEnable) {
    Serial.println("pomp1Enable   ");
    int count;
    int* days = parseDaysList(pompconfig.PompWorkWorkDays, count);

    int idx = day;

    bool dayOk =contains( days,count,day);
    bool timeOk = (pompconfig.PompStartHour == hour &&
                   pompconfig.PompStartMinute == minute);
  
    // Aynı dakikada bir kez tetikle
    if (dayOk && timeOk && lastRunMinute!=minute ) {
      lastRunMinute= minute;
    
      OpenPomp();       
      relayActive = true;
      relayStartTime = millis();
      relayDuration = (unsigned long)pompconfig.PompWorkingTime * 1000; // ms’ye çevir
      delay(relayDuration); 
      ClosePomp();


    } else if (!timeOk) {
      // dakika değiştiyse resetle ki gelecek dakikada tekrar tetiklenebilsin
      if (lastRunMinute != minute) 
          lastRunMinute = -1;
      // Serial.println("Not Time   Pomp 1");
    }
  }
 

  Serial.printf("%02d/%02d/%04d %02d:%02d:%02d - %s\n",
                timeinfo->tm_mday,
                timeinfo->tm_mon + 1,
                timeinfo->tm_year + 1900,
                hour, minute, timeinfo->tm_sec,
                gunler[day]);
}
