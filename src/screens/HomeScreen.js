import React, { useState, useEffect } from 'react';
import { View, Text, StyleSheet, ScrollView, RefreshControl, TouchableOpacity } from 'react-native';
import { useNavigation } from '@react-navigation/native';
import i18n from '../i18n'; // i18n yapılandırması import edilmeli
import { useTranslation } from 'react-i18next';
import { AuthContext } from '../navigation/AppNavigator';
import PlantSmallView from "../screens/devicescreen/plantvase/PlantSmallView"; // yeni ekran
import ErrorMessage from '../companent/ErrorMessage';

import auth from '@react-native-firebase/auth';
import firestore from '@react-native-firebase/firestore';

const HomeScreen = ({ navigation }) => {

    const [deviceList, setDeviceList] = useState([]);
    const [loading, setLoading] = useState(true);
    const [refreshing, setRefreshing] = useState(false);
    const [error, setError] = useState(null);
    const { t, i18n } = useTranslation();
    const getDeviceList = async () => {
        try {

            setRefreshing(true);
            var user = auth().currentUser;
            if (!user) {
                console.log('Henüz giriş yapılmamış');
                return;
            }

            const usersCollection = await firestore().collection('Device').where('userid', 'in', [user.uid]).get()
                .then((querySnapshot) => {
                    /*
                     A QuerySnapshot allows you to inspect the collection,
                     such as how many documents exist within it,
                     access to the documents within the collection,
                     any changes since the last query and more.
                 */
                    setDeviceList(documents => querySnapshot.docs.map(doc => ({ id: doc.id, ...doc.data() })));

                });

        } catch (error) {
            setError(error.message);
            console.error('Firestore veri çekme hatası:', error);
        } finally {
            setLoading(false);
            setRefreshing(false);
        }
    };

    useEffect(() => {
        getDeviceList();
    }, []);

    const CreateViewVaseV1 = (device) => {

        return (
            <TouchableOpacity
                key={device.id}
                onPress={() => navigation.navigate('PlantBigView',
                    { deviceid: device.deviceid, deviceType: device.devicetype,devicename:device.devicename })}
            >
                <PlantSmallView
                    key={device.id}
                    plantName={device.devicename}
                   // soilMoistureLevel={device.soilMoistureLevel}
                   // airHumidity={device.airHumidity}
                  //  temperature={device.temperature}
                    deviceid={device.deviceid}
                    deviceType={device.devicetype}

                /></TouchableOpacity >
        )
    }

    // const { setUserToken } = useContext(AuthContext);

    return (
        <ScrollView
            contentContainerStyle={{ padding: 20 }}
            refreshControl={
                <RefreshControl refreshing={refreshing} onRefresh={getDeviceList} />
            }
        >
            <ErrorMessage message={error} />
            {
                deviceList.map((device, index) => {
                    if (device.devicetype === 'SmartVaseV1') {
                        return CreateViewVaseV1(device)
                    } else {
                        return null; // Gösterme
                    }
                })
            }


        </ScrollView>
    );
};


export default HomeScreen;