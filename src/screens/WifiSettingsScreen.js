import React, { useState, useEffect } from 'react';
import { View, Text, TextInput, TouchableOpacity, PermissionsAndroid, Platform, StyleSheet, Alert } from 'react-native';
import Icon from 'react-native-vector-icons/Ionicons';
import WifiManager from "react-native-wifi-reborn";
import { useRoute } from '@react-navigation/native';
import i18n from '../i18n'; // i18n yapılandırması import edilmeli
import { useTranslation } from 'react-i18next';
import ErrorMessage from '../companent/ErrorMessage';
import LottieView from 'lottie-react-native';
import Config from 'react-native-config';
const WifiSettingsScreen = ({ navigation }) => {
  const { t, i18n } = useTranslation();
  const route = useRoute();
  const deviceType = route.params?.deviceType || 'V1'; // Varsayılan değer olarak 'V1' kullanılıyor
  const { devicessid = '', devicepassword = '' } = route.params || {};
  const { defaultSsid = '' } = route.params || {};


  const [ssidDevice, setssidDevice] = useState(devicessid);
  const [passwordDevice, setpasswordDevice] = useState(devicepassword);
  const [deviceName, setDeviceName] = useState(deviceType); // Yeni state değişkeni
  const [errorMessage, setErrorMessage] = useState(null);

  const [ssid, setSsid] = useState(defaultSsid);
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [deviceWifiConnected, setDeviceWifiConnected] = useState(false);

  useEffect(() => {

    requestPermissions();
  }, []);

  const handleConnect = () => {
    // Burada WiFi bilgilerini kaydetme ya da gönderme işlemleri yapılabilir
    // Alert.alert('Bağlantı', `SSID: ${ssid}\nŞifre: ${password}`);
  };
  const requestPermissions = async () => {
    if (Platform.OS === 'android') {
      try {
        const granted = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          PermissionsAndroid.PERMISSIONS.ACCESS_WIFI_STATE,
          PermissionsAndroid.PERMISSIONS.CHANGE_WIFI_STATE
        ]);
        console.log('Permissions:', granted);
      } catch (err) {
        console.warn(err);
      }
    }
  };
  const connectToWifi = () => {
    WifiManager.connectToProtectedSSID(ssidDevice, passwordDevice, false)
      .then(() => {
        //Alert.alert('Bağlantı Başarılı', `WiFi ağına bağlanıldı: ${ssidDevice}`);   
        setDeviceWifiConnected(true);
        //Config.mqtt_server = 'mqtts://m6e105d6.ala.eu-central-1.emqxsl.com';
      })
      .catch((error) => {
        console.log(error);
        setErrorMessage(t("WFConnectionError"));
        setDeviceWifiConnected(false);
        //  Alert.alert('Bağlantı Hatası', 'WiFi ağına bağlanılamadı.');
      });
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>{t("WifiSettings")}</Text>
      <Text style={styles.label}>{t("WFSettingsSSID")}</Text>
      <View style={styles.passwordContainer}>


        <TextInput
          style={[styles.input, { flex: 1 }]}
          placeholder={t("WFSettingsSSID")}
          value={ssid}
          onChangeText={setSsid}
        />
        <TouchableOpacity onPress={() => navigation.navigate("WifiScanner")}>
          <Icon
            name='wifi-outline'
            size={24}
            color="gray"
            style={{ marginLeft: 8 }}
          />
        </TouchableOpacity>

      </View>
      <Text style={styles.label}>{t("password")}</Text>

      <View style={styles.passwordContainer}>
        <TextInput
          style={[styles.input, { flex: 1 }]}
          placeholder={t("password")}
          secureTextEntry={!showPassword}
          value={password}
          onChangeText={setPassword}
        />



        <TouchableOpacity onPress={() => setShowPassword(!showPassword)}>
          <Icon
            name={showPassword ? 'eye' : 'eye-off'}
            size={24}
            color="gray"
            style={{ marginLeft: 8 }}
          />
        </TouchableOpacity>
      </View>

      <View>
        <Text style={styles.label}>{t("DeviceName")}</Text>
        <TextInput
          style={styles.input}
          placeholder={t("DeviceName")}
          value={deviceName}
          onChangeText={setSsid}
        />
      </View>
      <ErrorMessage message={errorMessage}></ErrorMessage>
      {deviceWifiConnected &&
        (
          <LottieView source={require('../../assets/Animation_Connection.json')}
            autoPlay loop style={{ width: 150, height: 150, alignSelf: 'center'}} />

        )}


      <TouchableOpacity style={styles.button} onPress={connectToWifi}>
        <Text style={styles.buttonText}>{t("SetSettings")}</Text>
      </TouchableOpacity>
    </View>
  );
};

export default WifiSettingsScreen;

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
    backgroundColor: '#fff',
    justifyContent: 'center',
  },
  title: {
    fontSize: 28,
    fontWeight: 'bold',
    marginBottom: 30,
    textAlign: 'center',
  },
  label: {
    fontSize: 16,
    marginBottom: 6,
    marginTop: 12,
  },
  input: {
    borderWidth: 1,
    borderColor: '#ccc',
    borderRadius: 10,
    padding: 10,
    fontSize: 16,
  },
  passwordContainer: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  button: {
    backgroundColor: '#007AFF',
    padding: 14,
    borderRadius: 10,
    marginTop: 30,
  },
  buttonText: {
    color: '#fff',
    fontSize: 18,
    textAlign: 'center',
  },
});
