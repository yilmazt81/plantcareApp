#include <FS.h>
#include <ArduinoJson.h>

struct PompSaveCommand { 
  bool pompEnable;
  int PompStartHour;
  int PompStartMinute;  
  String  PompRepeatType ;  
  String  PompWorkWorkDays;
};


PompSaveCommand pompconfig;

bool loadPompConfig() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS başlatılamadı.");
    return false;
  }
  if (!SPIFFS.exists("/deviceSetting.json")) {
    //Serial.println(" deviceSetting Config dosyası bulunamadı.");
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

  pompconfig.pompEnable = doc["pEn"].as<bool>() | false; 
  pompconfig.PompStartHour = doc["PSH"].as<int>();
  pompconfig.PompStartMinute = doc["PSM"].as<int>();
  pompconfig.PompRepeatType = doc["PRT"].as<String>();
  pompconfig.PompWorkWorkDays = doc["PWWD"].as<String>();

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