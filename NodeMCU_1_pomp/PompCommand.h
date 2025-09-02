#include <FS.h>
#include <ArduinoJson.h>

struct PompSaveCommand { 
  bool pompEnable;  
  bool enableLocation;
  int PompStartHour;
  int PompWorkingTime;
  int PompStartMinute;   
   
  String  PompRepeatType;
  String  PompWorkWorkDays; 
  String devicelatitude;
  String devicelongitude;
  
  
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




  pompconfig.PompWorkingTime = doc["PWT"].as<int>();  

  pompconfig.devicelatitude = doc["DLat"].as<String>();
  pompconfig.devicelongitude = doc["DLong"].as<String>();
  pompconfig.enableLocation=doc["pEn"].as<bool>() | false;


  Serial.println("Config yüklendi.");
  return true;
}



bool savePompConfig(StaticJsonDocument<1024>  doc) {
 
 
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