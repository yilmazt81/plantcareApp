import React from 'react';
import { View, Text, StyleSheet } from 'react-native';
import { useNavigation } from '@react-navigation/native';

import { AuthContext } from '../navigation/AppNavigator';


const HomeScreen = ({ navigation }) => {

   // const { setUserToken } = useContext(AuthContext);

    return (
        <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center' }}>
            <Text>Home Screen</Text>
        </View>
    );
};


export default HomeScreen;