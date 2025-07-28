

 Node mcu yu reset edildiğinde Wifi server modunda çalışacak , kendisi aşağıdaki bilgilerler Wifi yayını yapacak. 

 Wifi Name : "smartVasewf" 
 şifre : "12345678";

 bu kullanıcı adı ve şifre ile wifiye bağlanıp aşağıdaki post request i geçeceksiniz. kendi ağınızın bilgilerini geçeceksiniz.


 http://192.168.4.1/save
 {
    "wifi_ssid": "xxxx",
    "wifi_password": "xxxxxx",
    "mqtt_server":"m6e105d6.ala.eu-central-1.emqxsl.com",
    "mqtt_port":"8883",
    "mqtt_user":"waterUserName",
    "mqtt_password":"usr_26f924f3d92", 
    "deviceid":"hP3t8DuEs2"
  
}

bundan sonra alet restart olacak ve evin WF sine bağlanacak ve mqtt server a bilgi göndermeye başlayacak .

https://mqttx.app/downloads

buradaki uygulama ile aynı ayarları yapıldığında gelen mesajlar gözükür.


https://mqttx.app/downloads
