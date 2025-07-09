import React, { useEffect, useState } from 'react';
import {
  View,
  Text,
  FlatList,
  PermissionsAndroid,
  Platform,
  TouchableOpacity,
  Button,
  StyleSheet,
  Alert,
} from 'react-native';
import { useNavigation } from '@react-navigation/native';
import WifiManager from 'react-native-wifi-reborn';
import MaterialCommunityIcons from 'react-native-vector-icons/MaterialCommunityIcons';

const WifiScannerScreen = () => {
  const [wifiList, setWifiList] = useState([]);
  const navigation = useNavigation();
  const [error, setError] = useState(null);

  // Gerekli izinleri kullanıcıdan iste
  const requestLocationPermission = async () => {
    if (Platform.OS === 'android') {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        {
          title: 'Konum İzni Gerekli',
          message: 'WiFi taraması için konum izni gerekiyor.',
          buttonNeutral: 'Daha sonra sor',
          buttonNegative: 'İptal',
          buttonPositive: 'İzin Ver',
        }
      );
      return granted === PermissionsAndroid.RESULTS.GRANTED;
    }
    return true;
  };

  const loadWifiList = async () => {
    const hasPermission = await requestLocationPermission();
    if (!hasPermission) {
      console.warn('Konum izni verilmedi.');
      return;
    }

    WifiManager.loadWifiList()
      .then((networks) => {
        console.log('WiFi listesi:', networks);
        const sortedList = networks.sort((a, b) => b.level - a.level);

        setWifiList(sortedList);
      })
      .catch((error) => {
        console.error('WiFi listesi alınamadı:', error);
      });
  };

  useEffect(() => {
    loadWifiList();
  }, []);

  const getSignalIcon = (level) => {
    // Android'de sinyal seviyesi genellikle -30 (çok iyi) ile -90 (çok kötü) arası olur
    if (level >= -50) return 'wifi-strength-4';
    if (level >= -60) return 'wifi-strength-3';
    if (level >= -70) return 'wifi-strength-2';
    if (level >= -80) return 'wifi-strength-1';
    return 'wifi-strength-outline';
  };

  // Bir SSID seçildiğinde ayar ekranına yönlendir
  const handleSelect = (ssid) => {
    navigation.navigate('WifiSettings', {
      defaultSsid: ssid,
    });
  };

  return (
    <View style={styles.container}>
      <Button title="Yeniden Tara" onPress={loadWifiList} />

      {error && <Text style={styles.error}>{error}</Text>}
      <FlatList
        data={wifiList}
        keyExtractor={(item, index) => item.BSSID || index.toString()}
        renderItem={({ item }) => (

          <View style={styles.row}>
            <TouchableOpacity style={styles.item} onPress={() => handleSelect(item.SSID)}>
              <View>
                <MaterialCommunityIcons
                  name={getSignalIcon(item.level)}
                  size={24}
                  color="#333"
                  style={styles.icon}
                />
                <Text style={styles.ssid}>{item.SSID}</Text>

              </View>

            </TouchableOpacity>
          </View>
        )}
      />
    </View>
  );
};

export default WifiScannerScreen;

const styles = StyleSheet.create({
  container: { flex: 1, padding: 20, backgroundColor: '#fff' },
  item: {
    padding: 12,
    borderBottomWidth: 1,
    borderColor: '#ddd',
  },
  ssid: { fontSize: 18, fontWeight: 'bold' },
  signal: { color: 'gray' },
  error: { color: 'red', marginVertical: 30 },
});
