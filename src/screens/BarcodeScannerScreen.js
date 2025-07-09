import React, { useState, useEffect } from 'react';
import { View, Alert, StyleSheet, PermissionsAndroid, Platform, Text, Button } from 'react-native';
import { CameraType, Camera } from 'react-native-camera-kit'

export default function BarcodeScannerScreen({ navigation }) {
  const [scanned, setScanned] = useState(false);
  const [ssid, setSsid] = useState('test');
  const [password, setPassword] = useState('test1234');
  const [deviceType, setdeviceType] = useState("V1");

  const [barcodeValue, setBarcodeValue] = useState('');


  useEffect(() => {


    requestPermission();
  }, []);

  const onReadCode = (event) => {
    // Alert.alert('QR Kod Okundu');
    console.log(event);
    setBarcodeValue(event.nativeEvent.codeStringValue);
    var splitParts = event.nativeEvent.codeStringValue.split('-');
   
    setdeviceType(splitParts[0]);//DeviceType
    setSsid(splitParts[1]); //SSID
    setPassword(splitParts[2]); //Password

    setScanned(true); // kod okundu, tekrar okutmayı önlemek için
     navigation.navigate("WifiSettings", { deviceType: deviceType, devicessid: ssid, devicepassword: password });

  };

  const requestPermission = async () => {
    if (Platform.OS === 'android') {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.CAMERA,
        {
          title: 'Kamera Erişimi Gerekli',
          message: 'QR kodları okumak için kamera izni gerekiyor.',
          buttonNeutral: 'Daha Sonra',
          buttonNegative: 'İptal',
          buttonPositive: 'Tamam',
        }
      );
      setIsPermitted(granted === PermissionsAndroid.RESULTS.GRANTED);
    } else {
      setIsPermitted(true);
    }
  };
 
  return (
    <View>
      <Camera
        // Barcode props
        scanBarcode={true}
        onReadCode={(event) => onReadCode(event)} // optional
        showFrame={true} // (default false) optional, show frame with transparent layer (qr code or barcode will be read on this area ONLY), start animation for scanner, that stops when a code has been found. Frame always at center of the screen
        laserColor='red' // (default red) optional, color of laser in scanner frame
        frameColor='white' // (default white) optional, color of border of scanner frame
        style={{ width: '100%', height: '80%' }}
      />
      <Text>{barcodeValue}</Text>
      <Button
        title="WF Okudum"
        onPress={() => {
          navigation.navigate("WifiSettings", { deviceType: deviceType, devicessid: ssid, devicepassword: password });
        }}></Button>
    </View>
  );
}
