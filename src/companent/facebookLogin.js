import React from 'react';
import { View, Text, TextInput, TouchableOpacity, StyleSheet, Image } from 'react-native';
 
import { LoginManager, AccessToken } from 'react-native-fbsdk-next';
import { AuthContext } from '../navigation/AppNavigator';
import { useTranslation } from 'react-i18next';
export default function FacebookLoginButton() {
    const { setUserToken } = React.useContext(AuthContext);
    const { t, i18n } = useTranslation();
    const handleFacebookLogin = async () => {
        try {
            /*const result = await LoginManager.logInWithPermissions(['public_profile', 'email']);

            if (result.isCancelled) {
                throw 'Kullanıcı girişi iptal etti';
            }

            const data = await AccessToken.getCurrentAccessToken();

            if (!data) {
                throw 'Facebook erişim token alınamadı';
            }

            const facebookCredential = auth.FacebookAuthProvider.credential(data.accessToken);
            const userCredential = await auth().signInWithCredential(facebookCredential);

            setUserToken(userCredential.user.uid); // Oturum açıldı
            */
        } catch (error) {
            alert(error.toString());
        }
    };

    return (
        <TouchableOpacity style={styles.socialButton} onPress={() => handleFacebookLogin()}   >
            <Icon name="facebook" size={20} color="white" />
            <Text style={styles.socialText}>{t("loginWithFacebook")}</Text>
        </TouchableOpacity>

    )
}
