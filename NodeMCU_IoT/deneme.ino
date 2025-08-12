#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
// #include "ConfigManager.h" 
#include <DHT11.h>


//#include <certs.h>

#define SOIL_SENSOR_PIN A0
// #define RESET_BUTTON_PIN D3 
// #define LED_BUILTIN D2
// #define HUMIDITY_BUILTIN D4

// unsigned long buttonPressStart = 0;
// const unsigned long resetHoldTime = 5000;  // 5 second


ESP8266WebServer server(80);

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient); 
DHT11 dht11(HUMIDITY_BUILTIN);
 
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
  WiFi.softAP("smartVasewf", "12345678");
  Serial.println("Config AP opening.");
  server.serveStatic("/", SPIFFS, "/index.html");
  server.on("/save", HTTP_POST, handleConfigSave);
  server.begin();
}

void connectWiFi() {
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
  Serial.print("WiFi connecting");
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    if (millis() - timeout > 15000) break;
  }

  Serial.println("");
  Serial.println("WiFi bağlı!");
  Serial.print("IP adresi: ");
  Serial.println(WiFi.localIP());  
}

 

void connectMQTT() {


  mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  mqttClient.setCallback(callback); 
  while (!mqttClient.connected()) {
 
    Serial.print("MQTT broker connectiong...");
    Serial.print(" Device Id ");
    Serial.print(config.deviceid.c_str());
    /*Serial.print(" ");

    Serial.print(config.mqtt_user.c_str());
    Serial.print(" ");
    Serial.print(config.mqtt_password.c_str());

    Serial.print(" Server ");
    Serial.print(config.mqtt_server.c_str());


    Serial.print(" Port ");
    Serial.print(config.mqtt_port);
    */

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

void publishSensor() {
  int sensorValue = analogRead(SOIL_SENSOR_PIN);
  int temperature = 0;
  int humidity = 0;

    // Attempt to read the temperature and humidity values from the DHT11 sensor.
//   int result = dht11.readTemperatureHumidity(temperature, humidity);

  float percent = map(sensorValue, 1023, 0, 0, 100);

  StaticJsonDocument<256> payload;
  payload["deviceid"] = config.deviceid;
  payload["soil_moisture"] = percent;
  payload["temperature"]=temperature;
  payload["humidity"]=humidity;

  String stringThree = String(config.deviceid)  + "/sensorData";
  
  char buffer[400];
  serializeJson(payload, buffer);
  mqttClient.publish(stringThree.c_str(), buffer);
  Serial.print("MQTT publish: ");
  Serial.println(buffer);
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mesaj geldi [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  digitalWrite(LED_BUILTIN,   HIGH);
  Serial.println();
}

void setupOTA() {
  ArduinoOTA.setHostname(config.deviceid.c_str());
  ArduinoOTA.begin();
  Serial.println("OTA ready.");
}

void setup() {

  Serial.begin(115200); 
  
  /*
    SPIFFS.begin();
    SPIFFS.remove("/config.json");
    SPIFFS.end();

  */

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); //reset button
  pinMode(LED_BUILTIN, OUTPUT); //reset info led
  digitalWrite(LED_BUILTIN, HIGH);  // let close

  if (!loadConfig()) {
    startWebConfig();
    return;
  }

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
 // setupOTA();


  
}

void loop() {
   //Serial.println("loop ");
   server.handleClient();
  if (!mqttClient.connected()) {
      connectMQTT();
    }
  mqttClient.loop();

  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 60000) {
    publishSensor();
    lastPublish = millis();
  }
//Press reset button
if (digitalRead(RESET_BUTTON_PIN) == LOW) {
  Serial.println("Pressed ");
  if (buttonPressStart == 0) {
    buttonPressStart = millis();
  }
  digitalWrite(LED_BUILTIN, millis() % 500 < 250 ? LOW : HIGH);  // Aktif düşük
  if (millis() - buttonPressStart >= resetHoldTime) {
    Serial.println("The button was held for 5 seconds! Settings are resetting....");

    SPIFFS.begin();
    SPIFFS.remove("/config.json");
    SPIFFS.end();

    delay(1000);
    ESP.restart();
  }
} else {
  // If the button is released, the counter is reset.
  buttonPressStart = 0;
}

}
