import React from 'react';
import { View, Text, TextInput, TouchableOpacity, StyleSheet, Image } from 'react-native';
 
import { LoginManager, AccessToken } from 'react-native-fbsdk-next';
import { AuthContext } from '../navigation/AppNavigator';
import { useTranslation } from 'react-i18next';
export default function googleLogin() {
    const { setUserToken } = React.useContext(AuthContext);
    const { t, i18n } = useTranslation();
   
    return (
        <TouchableOpacity style={[styles.socialButton, { backgroundColor: '#DB4437' }]}>
            <Icon name="google" size={20} color="white" />
            <Text style={styles.socialText}> {t("loginWithGoogle")}</Text>
        </TouchableOpacity>

    )
}
