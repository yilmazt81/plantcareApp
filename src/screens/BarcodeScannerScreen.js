import React, { useState, useEffect } from 'react';
import { View, Alert, StyleSheet, PermissionsAndroid, Platform, Text,Button } from 'react-native';
import { Camera, CameraType, CameraKitCameraScreen } from 'react-native-camera-kit';


export default function BarcodeScannerScreen({ navigation }) {
  const [scanned, setScanned] = useState(false);
  const [ssid, setSsid] = useState('test');
  const [password, setPassword] = useState('test1234');
  const [deviceType, setdeviceType] = useState("V1");
  useEffect(() => {


    requestCameraPermission(); 
  }, []);
  const requestCameraPermission = async () => {
    if (Platform.OS === 'android') {
      try {
        const granted = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.CAMERA
        );
        if (granted !== PermissionsAndroid.RESULTS.GRANTED) {
          Alert.alert('Kamera izni reddedildi');
        }
      } catch (err) {
        console.warn(err);
      }
    }
  };

 

 

  return (
    <View>
      <Camera
        scanBarcode={true}
        style={{ width: '100%', height: '80%' }}
        onReadCode={(event) => Alert.alert('QR code found')} // optional
        showFrame={true} // (default false) optional, show frame with transparent layer (qr code or barcode will be read on this area ONLY), start animation for scanner, that stops when a code has been found. Frame always at center of the screen
        laserColor='red' // (default red) optional, color of laser in scanner frame
        frameColor='white' // (default white) optional, color of border of scanner frame

      />
      <Button
        title="WF Okudum"
        onPress={() => {
          navigation.navigate("WifiSettings",{deviceType:deviceType, devicessid: ssid, devicepassword: password});
        }}></Button>
    </View>
  );
}
