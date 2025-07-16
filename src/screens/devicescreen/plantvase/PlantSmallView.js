import React, { useState, useEffect } from 'react';
import { View, Text, StyleSheet } from 'react-native';
import Icon from 'react-native-vector-icons/MaterialCommunityIcons';
import { AnimatedCircularProgress } from 'react-native-circular-progress';
import Config from 'react-native-config';
import i18n from '../../../i18n/'; // i18n yapılandırması import edilmeli
import { useTranslation } from 'react-i18next';
import mqtt from 'mqtt';



const getMoistureIcon = (level) => {

    switch (level) {
        case 'nemli':
            return { name: 'water', color: '#4fc3f7', label: 'Nemli' };
        case 'normal':
            return { name: 'water-outline', color: '#81c784', label: 'Normal' };
        case 'kuru':
        default:
            return { name: 'water-off', color: '#e57373', label: 'Kuru' };
    }
};


const getTemperatureColor = (temperature) => {
    if (temperature <= 0) return '#42a5f5'; // mavi
    if (temperature >= 40) return '#ef5350'; // kırmızı
    if (temperature <= 20) {
        // mavi → yeşil arası geçiş
        const ratio = temperature / 20;
        return interpolateColor('#42a5f5', '#66bb6a', ratio);
    } else {
        // yeşil → kırmızı arası geçiş
        const ratio = (temperature - 20) / 20;
        return interpolateColor('#66bb6a', '#ef5350', ratio);
    }
};

const interpolateColor = (color1, color2, factor) => {
    const hexToRgb = (hex) =>
        hex.match(/\w\w/g).map((c) => parseInt(c, 16));

    const rgbToHex = (rgb) =>
        '#' +
        rgb
            .map((x) => {
                const hex = Math.round(x).toString(16);
                return hex.length === 1 ? '0' + hex : hex;
            })
            .join('');

    const c1 = hexToRgb(color1);
    const c2 = hexToRgb(color2);

    const result = c1.map((c, i) => c + (c2[i] - c) * factor);
    return rgbToHex(result);
};

const PlantSmallView = ({ plantName, deviceid }) => {

    const { t, i18n } = useTranslation();

    const [message, setMessage] = useState(t("Connecting"));

    const [client, setClient] = useState(null);

    const [temperature, setTemperature] = useState(0);
    const [airHumidity, setAirHumidity] = useState(0);
    const [soil_moisture, setSoilMoisture] = useState(0);
    const [icon, seticon] = useState(null);
    const [connected, setConnected] = useState(false);

    //const [soilMoistureLevel, setsoilMoistureLevel] = useState('normal'); // 'nemli', 'normal', 'kuru'


    const getSoilMoistureLevel = (soilMoisture) => {
        var soilMoisText = "";
        if (soilMoisture < 30) {
            soilMoisText = 'kuru';
        } else if (soilMoisture < 70) {
            soilMoisText = 'normal';
        } else {
            soilMoisText = 'nemli';
        }

        return soilMoisText;
    };

    const connectMqtt = () => {
        var topic = deviceid + '/sensorData';


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
            debugger;
            if (topic === topic) {
                var jsonData = JSON.parse(msg.toString());
                setSoilMoisture(jsonData.soil_moisture);
                setTemperature(jsonData.temperature);
                setAirHumidity(jsonData.humidity);

                var icon_ = getMoistureIcon(getSoilMoistureLevel(jsonData.soil_moisture));
                seticon(icon_);
            }
        });

        client.on('error', err => {
            console.log('MQTT WSS HATA:', err);
            setMessage(t("ConnectionError"), err.message);
            setConnected(fail);
        });

        return () => {
            client.end();
        };

    }

    useEffect(() => {
        connectMqtt();


    }, []);

    return (
        <View style={styles.card}>
            {/* Sol */}
            <View style={styles.leftColumn}>
                <Text style={styles.plantName}>{plantName}</Text>

            </View>
            <View style={styles.middleColumn}>

                <Icon name={icon?.name} size={32} color={icon?.color} />
                <Text style={[styles.statusLabel, { color: icon?.color }]}>{t(icon?.label)} </Text>
            </View>


            {/* Orta - sıcaklık */}
            <View style={styles.middleColumn}>
                <Icon name="thermometer" size={28} color={getTemperatureColor(temperature)} />
                <Text style={[styles.temperatureText, { color: getTemperatureColor(temperature) }]}>
                    {temperature}°C
                </Text>
            </View>

            {/* Sağ */}
            <View style={styles.rightColumn}>
                <Text style={styles.gaugeLabel}>Hava Nemi</Text>
                <AnimatedCircularProgress
                    size={70}
                    width={8}
                    fill={airHumidity}
                    tintColor="#00e0ff"
                    backgroundColor="#e0e0e0"
                    rotation={0}
                >
                    {(fill) => (
                        <Text style={styles.percentageText}>{`${Math.round(fill)}%`}</Text>
                    )}
                </AnimatedCircularProgress>
            </View>            
        </View>
    );
};

const styles = StyleSheet.create({
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
    leftColumn: {
        flex: 1,
        alignItems: 'flex-start',
        gap: 4,
    },

    middleColumn: {
        alignItems: 'center',
        marginHorizontal: 10,
    },
    rightColumn: {
        alignItems: 'center',
        justifyContent: 'center',
    },
    statusLabel: {
        fontSize: 16,
        fontWeight: '500',
    },
    plantName: {
        fontSize: 18,
        fontWeight: '600',
        marginTop: 4,
        color: '#333',
    },
    gaugeLabel: {
        fontSize: 14,
        marginBottom: 6,
        color: '#555',
    },
    percentageText: {
        fontSize: 16,
        fontWeight: 'bold',
        color: '#00bcd4',
    },
    temperatureText: {
        fontSize: 18,
        fontWeight: 'bold',
        color: '#ff7043',
        marginTop: 4,
    },
});

export default PlantSmallView;
