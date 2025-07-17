import React, { useState, useEffect } from 'react';
import { View, Text, Image, StyleSheet, TouchableOpacity, Dimensions, Alert, ScrollView } from 'react-native';

import Icon from 'react-native-vector-icons/MaterialCommunityIcons';
import { Button } from 'react-native-paper';
import { launchCamera, launchImageLibrary } from 'react-native-image-picker';
import Config from 'react-native-config';
import { useTranslation } from 'react-i18next';
import mqtt from 'mqtt';
import { getMoistureIcon, getSoilMoistureLevel } from './iconfunctions';
import StatusCard from './StatusCard';
import { useRoute } from '@react-navigation/native';

//import storage from '@react-native-firebase/storage';
import { v2 as cloudinary } from 'cloudinary';


import ErrorMessage from '../../../companent/ErrorMessage';

const screenWidth = Dimensions.get("window").width;


const PlantBigView = () => {
    const route = useRoute();
    const [response, setResponse] = useState(null);
    const [uploading, setUploading] = useState(false);

    const { t, i18n } = useTranslation();
    const { deviceid = '', devicename = '' } = route.params || {};
    const [message, setMessage] = useState(t("Connecting"));
    const [errorMessage, setErrorMessage] = useState(null);

    const [client, setClient] = useState(null);

    const [temperature, setTemperature] = useState(0);
    const [airHumidity, setAirHumidity] = useState(0);
    const [soil_moisture, setSoilMoisture] = useState(0);
    const [icon, seticon] = useState(null);
    const [connected, setConnected] = useState(false);
    const [imageUri, setImageUri] = useState(null);
    const [transferred, setTransferred] = useState(0);



    const onPictureButtonPress = () => {
        Alert.alert(
            "Resim Kaynağı",
            "Fotoğrafı nasıl eklemek istersin?",
            [
                {
                    text: "Kamera",
                    onPress: () => openCamera()
                },
                {
                    text: "Galeri",
                    onPress: () => openGallery()
                },
                { text: "İptal", style: "cancel" }
            ]
        );
    };
    const openCamera = () => {
        launchCamera({ mediaType: 'photo' }, (response) => {
            if (response.didCancel || !response.assets) return;
            const uri = response.assets[0].uri;
            setImageUri(uri);
            uploadImage(uri);
        });
    };

    const openGallery = () => {
        launchImageLibrary({ mediaType: 'photo' }, (response) => {
            if (response.didCancel || !response.assets) return;
            const uri = response.assets[0].uri;
            setImageUri(uri);
            uploadImage(uri);
        });
    };


    const connectMqtt = () => {
        var topic = deviceid + '/sensorData';
        setErrorMessage(null);

        const client = mqtt.connect(Config.MQTTWebSocket, {
            port: Config.MQTTWebSocketPort,
            clientId: 'rn_client_' + Math.random().toString(16).substr(2, 8),
            username: Config.mqtt_username,    // eğer auth varsa
            password: Config.mqtt_password,    // eğer auth varsa
            rejectUnauthorized: false, // self-signed cert için
        });

        client.on('connect', () => {
            console.log('WSS MQTT bağlandı');
            setMessage(t("Connected"));
            setConnected(true);
            setClient(client);
            client.subscribe(topic);
        });

        client.on('message', (topic, msg) => {
            setMessage(null);
            if (topic === topic) {
                var jsonData = JSON.parse(msg.toString());
                setSoilMoisture(jsonData.soil_moisture);
                setTemperature(jsonData.temperature);
                setAirHumidity(jsonData.humidity);

                var icon_ = getMoistureIcon(getSoilMoistureLevel(jsonData.soil_moisture));
                console.log(icon_);
                seticon(icon_);
            }
        });

        client.on('error', err => {
            console.log('MQTT WSS HATA:', err);
            setErrorMessage(t("ConnectionError"), err.message);
            setConnected(false);
        });

        return () => {
            client.end();
        };

    }

    const StartPomp = async () => {
        if (!client) {
            console.warn('MQTT client henüz bağlanmadı.');
            return;
        }
        var topic = deviceid + '/command';

        const command = {
            command: 'water',
            value: 1,
        };

        client.publish(topic, JSON.stringify(command), { qos: 1 }, (error) => {
            if (error) {
                console.error('Publish Hatası:', error);
                setErrorMessage('Publish Hatası:', error);
            } else {
                console.log('Komut gönderildi:', command);
            }
        });
    };


    const uploadImage = async (uri) => {

        
        /* const filename = uri.substring(uri.lastIndexOf('/') + 1);
         const uploadUri = Platform.OS === 'ios' ? uri.replace('file://', '') : uri;
 
         setUploading(true);
         setTransferred(0);
 
         const task = storage()
             .ref(filename)
             .putFile(uploadUri);
         debugger;
         // set progress state
         task.on('state_changed', snapshot => {
             setTransferred(
                 Math.round(snapshot.bytesTransferred / snapshot.totalBytes) * 10000
             );
         });
 
         try {
             await task;
         } catch (e) {
             console.error(e);
             setErrorMessage(e.message);
         }
 
         setUploading(false);
         Alert.alert(
             'Photo uploaded!',
             'Your photo has been uploaded to Firebase Cloud Storage!'
         );
 
         setImageUri(null);
         */
    };

    useEffect(() => {
        connectMqtt();


    }, []);

    // Örnek veri (gerçek sensör verisiyle değiştirilebilir)
    /*const days = ["Pzt", "Sal", "Çar", "Per", "Cum", "Cumrt", "Paz"];
    const humidityData = [60, 65, 70, 75, 72, 70, 60];
    const temperatureData = [22, 24, 26, 25, 23, 25, 26];
    const soilMoistureData = [40, 45, 38, 50, 42, 34, 50];
    const chartConfig = {
        backgroundColor: "#e0f7fa",
        backgroundGradientFrom: "#e0f7fa",
        backgroundGradientTo: "#b2ebf2",
        decimalPlaces: 0,
        color: (opacity = 1) => `rgba(0, 150, 136, ${opacity})`,
        labelColor: () => '#00796b',
    };*/



    return (
        <ScrollView style={styles.container}>
            <View style={styles.imageContainer}>
                <Image
                    source={imageUri ? { uri: imageUri } : require('./Plant.jpg')}
                    style={styles.image}
                />
                <TouchableOpacity style={styles.editIcon} onPress={onPictureButtonPress}>
                    <Icon name="image-edit" size={28} color="#fff" />
                </TouchableOpacity>
            </View>

            <View style={styles.infoSection}>
                <Text style={styles.name}> {devicename} </Text>
                <Text style={styles.name}>Aloe Vera</Text>
                <Text style={styles.description}>Güneşli alanları seven, suyu depolayan bir bitki türüdür.</Text>
            </View>
            <StatusCard icon={icon}
                temperature={temperature}
                airHumidity={airHumidity}
                t={t}
            ></StatusCard>
            {/* Nem 
            <View style={styles.chartSection}>
                <Text style={styles.chartTitle}>Hava Nemi (%)</Text>
                <LineChart
                    data={{ labels: days, datasets: [{ data: humidityData }] }}
                    width={screenWidth - 40}
                    height={180}
                    chartConfig={chartConfig}
                    bezier
                    style={styles.chart}
                />
            </View>
 
            <View style={styles.chartSection}>
                <Text style={styles.chartTitle}>Sıcaklık (°C)</Text>
                <LineChart
                    data={{ labels: days, datasets: [{ data: temperatureData }] }}
                    width={screenWidth - 40}
                    height={180}
                    chartConfig={chartConfig}
                    bezier
                    style={styles.chart}
                />
            </View>
 
            <View style={styles.chartSection}>
                <Text style={styles.chartTitle}>Toprak Nem Oranı (%)</Text>
                <LineChart
                    data={{ labels: days, datasets: [{ data: soilMoistureData }] }}
                    width={screenWidth - 40}
                    height={180}
                    chartConfig={chartConfig}
                    bezier
                    style={styles.chart}
                />
            </View>
*/}
            <Button
                icon="water"
                mode="contained"
                onPress={StartPomp}
                style={styles.waterButton}
            >
                {t("StartWaterPomp")}
            </Button>
            <ErrorMessage message={errorMessage}></ErrorMessage>
        </ScrollView>
    );
}

export default PlantBigView;

const styles = StyleSheet.create({
    container: {
        flex: 1,
        backgroundColor: "#f4fdfd",
        padding: 20
    },
    card: {
        backgroundColor: '#f9f9f9',
        borderRadius: 16,
        padding: 10,
        flexDirection: 'row',
        alignItems: 'center',
        elevation: 3,
        marginVertical: 10,
        justifyContent: 'space-between',
    },
    imageContainer: {
        position: "relative",
        alignItems: "center",
        marginBottom: 20
    },
    image: {
        width: 160,
        height: 160,
        borderRadius: 80,
        borderWidth: 2,
        borderColor: "#009688"
    },
    editIcon: {
        position: "absolute",
        bottom: 10,
        right: screenWidth / 2 - 90,
        backgroundColor: "#009688",
        borderRadius: 20,
        padding: 6
    },
    infoSection: {
        alignItems: "center",
        marginBottom: 20
    },
    name: {
        fontSize: 24,
        fontWeight: "bold",
        color: "#00796b"
    },
    description: {
        fontSize: 14,
        color: "#555",
        textAlign: "center",
        marginTop: 5
    },
    chartSection: {
        marginBottom: 30
    },
    chartTitle: {
        fontSize: 16,
        fontWeight: "bold",
        marginBottom: 10,
        color: "#00796b"
    },
    chart: {
        borderRadius: 12
    },
    waterButton: {
        backgroundColor: "#00796b",
        marginHorizontal: 40,
        borderRadius: 10,
        marginBottom: 30
    },
    leftColumn: {
        flex: 1,
        alignItems: 'flex-start',
        gap: 2,
    },

    middleColumn: {
        alignItems: 'center',
        marginHorizontal: 10,
    },
    rightColumn: {
        alignItems: 'center',
        justifyContent: 'center',
    },
});

