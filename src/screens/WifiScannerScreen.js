import React, { useEffect, useState } from 'react';
import { View, Text, FlatList, PermissionsAndroid, Platform, Button } from 'react-native';
import WifiManager from 'react-native-wifi-reborn';

const WifiScannerScreen = () => {
  const [wifiList, setWifiList] = useState([]);

  const requestPermissions = async () => {
    if (Platform.OS === 'android') {
      const granted = await PermissionsAndroid.requestMultiple([
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        PermissionsAndroid.PERMISSIONS.ACCESS_WIFI_STATE,
      ]);
      return granted['android.permission.ACCESS_FINE_LOCATION'] === 'granted';
    }
    return true;
  };

  const scanWifi = async () => {
    const hasPermission = await requestPermissions();
    if (!hasPermission) {
      console.warn('İzin verilmedi.');
      return;
    }

    try {
      const wifiList = await WifiManager.reScanAndLoadWifiList();
      setWifiList(wifiList);
    } catch (error) {
      console.error('WiFi listesi alınamadı:', error);
    }
  };

  useEffect(() => {
    scanWifi();
  }, []);

  return (
    <View style={{ flex: 1, padding: 20 }}>
      <Button title="Yenile" onPress={scanWifi} />
      <FlatList
        data={wifiList}
        keyExtractor={(item) => item.BSSID}
        renderItem={({ item }) => (
          <View style={{ padding: 10, borderBottomWidth: 1 }}>
            <Text>{item.SSID}</Text>
            <Text style={{ color: 'gray' }}>Sinyal: {item.level}</Text>
          </View>
        )}
      />
    </View>
  );
};

export default WifiScannerScreen;
