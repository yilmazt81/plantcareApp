import React, { useState, useEffect } from 'react';
import { View, Alert, StyleSheet, PermissionsAndroid, Platform, Text } from 'react-native';
import { Camera, CameraType, CameraKitCameraScreen } from 'react-native-camera-kit';


export default function BarcodeScannerScreen({ navigation }) {
  const [scanned, setScanned] = useState(false);

  useEffect(() => {
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

    requestCameraPermission();
  }, []);



  return (
    <View>
      <Camera
        scanBarcode={true} 
        style={{width: '100%', height: '100%'}}
        onReadCode={(event) => Alert.alert('QR code found')} // optional
        showFrame={true} // (default false) optional, show frame with transparent layer (qr code or barcode will be read on this area ONLY), start animation for scanner, that stops when a code has been found. Frame always at center of the screen
        laserColor='red' // (default red) optional, color of laser in scanner frame
        frameColor='white' // (default white) optional, color of border of scanner frame

      />
    </View>
  );
}
