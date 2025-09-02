 
 
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

#define MQTT_LED_PING D5 // D1(gpio5)
#define MQTT_LED_PINR D6  // ya da #define LED_PIN 2

#define WIFI_LED_PING  D4 //Buna bak yanlis D4 olabilir
#define WIFI_LED_PINR  D3 


//#define LED_BUILTIN D2
//#define HUMIDITY_BUILTIN D4

const char* HOST = "api.open-meteo.com";
const int   HTTPS_PORT = 443;

#define Pomp_1 D7
#define Pomp_2 D1
 

 struct PumpState {
  bool running = false;
  int number=0;
  unsigned long startMs = 0;
  unsigned long durationMs = 0;
  int lastRunMinute = -1;  // Aynı dakikada tekrar tetiklemeyi engelle
};


void startPump(PumpState& p, unsigned long durationMs, const char* tag);
void handlePump(PumpState& p, const char* tag);
 PumpState pump1, pump2;

unsigned long buttonPressStart = 0;
unsigned long connectWifi=0;
const unsigned long resetHoldTime = 5000;  // 5 second
bool   relayOn            = false;
unsigned long relayOffAt  = 0; // millis zaman damgası
 

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

String urlEncode(const String& s) {
  String out; out.reserve(s.length()*3);
  const char *hex = "0123456789ABCDEF";
  for (size_t i=0;i<s.length();++i) {
    char c = s[i];
    if (('a'<=c && c<='z')||('A'<=c && c<='Z')||('0'<=c && c<='9')||c=='-'||c=='_'||c=='.'||c=='~') {
      out += c;
    } else {
      out += '%';
      out += hex[(c >> 4) & 0xF];
      out += hex[c & 0xF];
    }
  }
  return out;
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

  
void  OpenClosePomp(bool on,int pompNumber) {
  relayOn = on;
  if (pompNumber==1)
  {
   
      digitalWrite(Pomp_1, on ?  HIGH:LOW);
    
  }else if (pompNumber==2)
  {
  
      digitalWrite(Pomp_2, on ? HIGH:LOW);   
  }
 
  String  pub_topic=String(config.deviceid) + "/status";
  String message="pompstatus|"+String(pompNumber)+"|"+String(on);
  
  Serial.print(message); 

  mqttClient.publish(pub_topic.c_str(),message.c_str(), false);
  mqttClient.loop();
} 

String buildUrl() {


 /* String url =  "https://api.open-meteo.com/v1/forecast?latitude=";
  url +=urlEncode( pompconfig.devicelatitude);
  url += "&longitude=";
  url += urlEncode(pompconfig.devicelongitude);
  url += "&current="+urlEncode("temperature_2m,precipitation")+"&timezone=auto";
*/
 
// timezone=auto sunucu tarafında yerel saate göre saat dizilerini döndürür
String url = String("/v1/forecast?latitude=") + String(pompconfig.devicelatitude) +
             "&longitude=" + String(pompconfig.devicelongitude) +
             "&minutely_15=precipitation,rain&timezone=auto&forecast_days=1";


  return url;
}





// Aç ve belirli süre sonra otomatik kapat
/*void startRelayFor(unsigned long ms,int pompnumber) {
  setRelay(true,pompnumber);
  relayOffAt = millis() + ms;
}*/
void connectMQTT() {


  mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  mqttClient.setCallback(callback); 

  digitalWrite(MQTT_LED_PING,  LOW);
  digitalWrite(MQTT_LED_PINR,  LOW); 

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
      digitalWrite(MQTT_LED_PING,  HIGH);
      digitalWrite(MQTT_LED_PINR,  LOW); 
      mqttClient.subscribe(commandSubscribe.c_str());
    } else {
      Serial.print("cannot connect: ");
      digitalWrite(MQTT_LED_PING, LOW  );
      digitalWrite(MQTT_LED_PINR,  HIGH); 
      Serial.print(mqttClient.state());
      delay(2000); 
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
    Serial.printf("Komut: %s, Değer: %d, Süre: %d\n Pomp: %d\n", command, value, time,pomp);
    
    PumpState& target = (pomp == 2 ? pump2 : pump1);
       
    if (value == 1)
    {
        Serial.println("Röle çalıştırılıyor...");
        const char* label = (pomp == 2 ? "Pomp 2" : "Pomp 1");
        target.number = pomp;
        startPump(target, (unsigned long)time * 1000UL, label);
       
        relayActive = true;
        relayStartTime = millis();
        relayDuration = (unsigned long)time * 1000; // ms’ye çevir
        delay(relayDuration); 
        handlePump(target, label);
        Serial.println("Röle Durduruluyor...");
    }else
    {
       // Serial.println("Röle Durduruluyor...");
        OpenClosePomp(false, pomp);
        target.running = false;
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
  
  
  pinMode(Pomp_2, OUTPUT);
  pinMode(Pomp_1, OUTPUT);
  
  pinMode(RESET_BUTTON_PIN, INPUT); 

  digitalWrite(Pomp_2,  LOW);
  digitalWrite(Pomp_1,  LOW);  // Aktif düşük


  pinMode(MQTT_LED_PING, OUTPUT);
  pinMode(MQTT_LED_PINR, OUTPUT);

  pinMode(WIFI_LED_PING, OUTPUT);
  pinMode(WIFI_LED_PINR, OUTPUT);

  digitalWrite(WIFI_LED_PING,  LOW);
  digitalWrite(WIFI_LED_PINR,  LOW);

  digitalWrite(MQTT_LED_PING,  LOW);
  digitalWrite(MQTT_LED_PINR,  LOW);  // Aktif düşük

 

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
 
bool Getforecast() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi yok");
    return false;
  }
 
  float rainNow= 0.0f;
  float rainNowMm0=0.0f;
  float rainNowMm=0.0f;

  WiFiClientSecure httpClientforecast;
  httpClientforecast.setInsecure(); // test için; üretimde CA ekleyin

 
 String url=buildUrl();
 String req =

    String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + HOST + "\r\n" +
    "User-Agent: ESP-Weather/1.0\r\n" +
    "Connection: close\r\n\r\n";

  httpClientforecast.print(req);



  if (!httpClientforecast.find("\r\n\r\n")) {
    Serial.println("HTTP header okunamadı!");
    return false;
  }

    // Gövdeyi topla
  String payload;
  while (httpClientforecast.connected() || httpClientforecast.available()) {
    String chunk = httpClientforecast.readString();
    if (chunk.length() == 0) break;
    payload += chunk;
  }


StaticJsonDocument<32 * 1024> doc; // Open-Meteo JSON'u büyük olabilir
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("JSON parse hatası: ");
    Serial.println(err.f_str());
    return false;
  }

  JsonObject m15 = doc["minutely_15"];
  if (m15.isNull()) {
    Serial.println("minutely_15 bulunamadı!");
    return false;
  }

  JsonArray times = m15["time"].as<JsonArray>();
  JsonArray precip = m15["precipitation"].as<JsonArray>(); // mm
  JsonArray rain   = m15["rain"].as<JsonArray>();           // mm (yağmur bileşeni)

  if (times.isNull() || (precip.isNull() && rain.isNull())) {
    Serial.println("Beklenen alanlar yok!");
    return false;
  }

  // Basit strateji: dizinin ilk elemanını "şimdi/çok yakın" kabul et
  // (Open-Meteo 15 dk dilimlerini 'şimdi'ye hizalı döndürür)
  int idx = 0;
  float valPrecip = precip.isNull() ? 0.0f : precip[idx] | 0.0f;
  float valRain   = rain.isNull()   ? 0.0f : rain[idx]   | 0.0f;

  // "rain" varsa onu kullan, yoksa "precipitation" (kar, karla karışık yağmur dahil)
  rainNowMm = (valRain > 0.0f) ? valRain : valPrecip;

  // Bilgi amaçlı log
  String t = times[idx] | "";
  Serial.print("Zaman: "); Serial.print(t);
  Serial.print(" | rain(mm): "); Serial.print(valRain, 3);
  Serial.print(" | precipitation(mm): "); Serial.print(valPrecip, 3);
  Serial.println();

  return true;
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
  /*
  //Press reset button
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {

        Serial.println("Pressed ");
        
        if (buttonPressStart == 0) {
          buttonPressStart = millis();
        }

        digitalWrite(MQTT_LED_PING, millis() % 500 < 250 ? LOW : HIGH);
        digitalWrite(MQTT_LED_PINR, millis() % 500 < 250 ? HIGH : LOW);  // Aktif düşük

        if (millis() - buttonPressStart >= resetHoldTime) {

          SPIFFS.begin();
          SPIFFS.remove("/config.json");
          SPIFFS.remove("/deviceSetting.json");          
          SPIFFS.end();
          Serial.println("The button was held for 5 seconds! Settings are resetting....");
          digitalWrite(MQTT_LED_PING,  LOW);
          digitalWrite(MQTT_LED_PINR,  LOW);  // Aktif düşük

          digitalWrite(WIFI_LED_PING,  LOW);        
          digitalWrite(WIFI_LED_PING,  LOW);
        
          delay(1000);
          ESP.restart();

        }
      
    } else {
        // If the button is released, the counter is reset.
        buttonPressStart = 0;
    }
    */
 
     
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

 


 void handlePump(PumpState& p, const char* tag) {
  if (p.running) {
    if (millis() - p.startMs >= p.durationMs) {
      p.running = false;
      // burada röleyi KAPAT
      // digitalWrite(RELAY_PIN, LOW);
      OpenClosePomp(false,p.number); 
      Serial.printf("%s tamamlandi\n", tag);
    }
  }
}



void startPump(PumpState& p, unsigned long durationMs, const char* tag) {
  if (!p.running) {
    p.running = true;
    p.startMs = millis();
    p.durationMs = durationMs;
    Serial.printf("Run %s\n", tag);

    OpenClosePomp(true,p.number);
    // burada röleyi AÇ
    // digitalWrite(RELAY_PIN, HIGH);
  }
}

bool contains(int arr[], int size, int value) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == value) {
      return true;  // bulundu
    }
  }
  return false; // yok
}


void CheckDateTimeForWork() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  if (!timeinfo) {
    Serial.println("Zaman bilgisi alinamadi");
    return;
  }

  const char* gunler[] = { "Pazar","Pazartesi","Salı","Çarşamba","Perşembe","Cuma","Cumartesi" };
  int hour   = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  int day    = timeinfo->tm_wday; // 0=Pazar ... 6=Cumartesi


  
  // --- POMP 1 ---
  if (pompconfig.pomp1Enable) {
     Serial.println("pomp1Enable   ");
    int count;
    int* days = parseDaysList(pompconfig.PompWork1WorkDays, count);

    // Eğer masken Pazartesi ile başlıyorsa şu satırı kullan:
    // int w = (day + 6) % 7;
    // else doğrudan day kullan:
    int idx = day;

    bool dayOk =contains( days,count,day);
    bool timeOk = (pompconfig.Pomp1StartHour == hour &&
                   pompconfig.Pomp1StartMinute == minute);
    // Serial.println("timeOk   ");
      //Serial.println(timeOk);

        //Serial.println("dayOk   ");
     // Serial.println(dayOk);

    // Aynı dakikada bir kez tetikle
    if (dayOk && timeOk && pump1.lastRunMinute!=minute ) {
      pump1.lastRunMinute = minute;
      pump1.number=1;
      
      startPump(pump1, (unsigned long)pompconfig.Pomp1WorkingTime * 1000UL, "Pomp 1");
    
        
      long relayDuration = (unsigned long)pompconfig.Pomp1WorkingTime * 1000; // ms’ye çevir
      delay(relayDuration); 
      handlePump(pump1, "Pomp 1");

    } else if (!timeOk) {
      // dakika değiştiyse resetle ki gelecek dakikada tekrar tetiklenebilsin
      if (pump1.lastRunMinute != minute) pump1.lastRunMinute = -1;
      // Serial.println("Not Time   Pomp 1");
    }
  }

  // --- POMP 2 ---
  if (pompconfig.pomp2Enable) {
    int count;
    int* days = parseDaysList(pompconfig.PompWork2WorkDays, count);
    int idx = day; // veya Pazartesi başlangıçlı maske için (day + 6) % 7

    bool dayOk =contains( days,count,day);
    bool timeOk = (pompconfig.Pomp2StartHour == hour &&
                   pompconfig.Pomp2StartMinute == minute);

    if (dayOk && timeOk && pump2.lastRunMinute!=minute) {
      pump2.lastRunMinute = minute;
      pump2.number=2; 
      
      startPump(pump2, (unsigned long)pompconfig.Pomp2WorkingTime * 1000UL, "Pomp 2");
      
      long relayDuration = (unsigned long)pompconfig.Pomp2WorkingTime * 1000; // ms’ye çevir
      delay(relayDuration); 
      handlePump(pump2, "Pomp 2");

    } else if (!timeOk) {
      if (pump2.lastRunMinute != minute) pump2.lastRunMinute = -1;
      // Serial.println("Not Time   Pomp 2");
    }
  }

  // Çalışan pompaları non-blocking şekilde yönet
  
  

  Serial.printf("%02d/%02d/%04d %02d:%02d:%02d - %s\n",
                timeinfo->tm_mday,
                timeinfo->tm_mon + 1,
                timeinfo->tm_year + 1900,
                hour, minute, timeinfo->tm_sec,
                gunler[day]);
}
