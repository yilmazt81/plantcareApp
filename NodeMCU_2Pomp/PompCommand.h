#include <FS.h>
#include <ArduinoJson.h>

struct PompSaveCommand { 
  bool pomp1Enable;
  bool pomp2Enable;

  int Pomp1StartHour;
  int Pomp1StartMinute;  
  int Pomp2StartHour;  
  int Pomp2StartMinute;

  String  Pomp2RepeatType ;  
  String  Pomp1RepeatType;
  String  PompWork1WorkDays;
  String  PompWork2WorkDays;
  
  
};


PompSaveCommand pompconfig;

bool loadPompConfig() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS başlatılamadı.");
    return false;
  }
  if (!SPIFFS.exists("/deviceSetting.json")) {
    Serial.println(" deviceSetting Config dosyası bulunamadı.");
    return false;
  }

  File configFile = SPIFFS.open("/deviceSetting.json", "r");
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

  pompconfig.pomp1Enable = doc["p1En"].as<bool>() | false;
  pompconfig.pomp2Enable = doc["p2En"].as<bool>() | false;

  pompconfig.Pomp1StartHour = doc["P1SH"].as<int>();
  pompconfig.Pomp1StartMinute = doc["P1SM"].as<int>();

  pompconfig.Pomp2StartHour = doc["P2SH"].as<int>();  
  pompconfig.Pomp2StartMinute = doc["P2SM"].as<int>();

  pompconfig.Pomp1RepeatType = doc["P1RT"].as<String>();
  pompconfig.Pomp2RepeatType = doc["P2RT"].as<String>();

  pompconfig.PompWork1WorkDays = doc["PW1WD"].as<String>();
  pompconfig.PompWork2WorkDays = doc["PW2WD"].as<String>();

  Serial.println("Config yüklendi.");
  return true;
}



bool savePompConfig(StaticJsonDocument<1024>  doc) {
 
 /* doc["pomp1Enable"] = pompconfig.pomp1Enable;
  doc["pomp2Enable"] = pompconfig.pomp2Enable;
  doc["Pomp1StartHour"] = pompconfig.Pomp1StartHour;
  doc["Pomp1StartMinute"] = pompconfig.Pomp1StartMinute;
  doc["Pomp2StartHour"] = pompconfig.Pomp2StartHour;
  doc["Pomp2StartMinute"] = pompconfig.Pomp2StartMinute;
  doc["Pomp2RepeatType"] = pompconfig.Pomp2RepeatType;
  doc["Pomp1RepeatType"] = pompconfig.Pomp1RepeatType;
*/
  File configFile = SPIFFS.open("/deviceSetting.json", "w");
  if (!configFile) {
    Serial.println("Config dosyası yazılamadı.");
    return false;
  }

  serializeJson(doc, configFile);
  configFile.close();
  Serial.println("Config kaydedildi.");
  return true;
}