import React, { useState } from 'react';
import { View, Text, TextInput, Button, Alert } from 'react-native';
import WifiManager from "react-native-wifi-reborn";

const WifiConnectScreen = () => {
  const [ssid, setSsid] = useState('');
  const [password, setPassword] = useState('');

  const connectToWifi = () => {
    WifiManager.connectToProtectedSSID(ssid, password, false)
      .then(() => {
        Alert.alert('Bağlantı Başarılı', `WiFi ağına bağlanıldı: ${ssid}`);
      })
      .catch((error) => {
        console.log(error);
        Alert.alert('Bağlantı Hatası', 'WiFi ağına bağlanılamadı.');
      });
  };

  return (
    <View style={{ padding: 20 }}>
      <Text>SSID (WiFi Adı):</Text>
      <TextInput
        value={ssid}
        onChangeText={setSsid}
        placeholder="WiFi Adı"
        style={{ borderWidth: 1, marginBottom: 10 }}
      />
      <Text>Şifre:</Text>
      <TextInput
        value={password}
        onChangeText={setPassword}
        placeholder="Şifre"
        secureTextEntry
        style={{ borderWidth: 1, marginBottom: 10 }}
      />
      <Button title="Bağlan" onPress={connectToWifi} />
    </View>
  );
};

export default WifiConnectScreen;
