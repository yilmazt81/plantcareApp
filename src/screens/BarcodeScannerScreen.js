import React, { useState, useEffect } from 'react';
import { View, Alert, StyleSheet, PermissionsAndroid, Platform,Text } from 'react-native';
import { CameraKitCameraScreen } from 'react-native-camera-kit';
 

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

  const onBarcodeRead = (event) => {
    if (scanned) return;
    setScanned(true);
    Alert.alert('Barkod', `Kod: ${event.nativeEvent.codeStringValue}`);
    navigation.goBack();
  };

  return (
    <View style={{ flex: 1 }}>
          <Text>Home Screen</Text>
    </View>
  );
}
