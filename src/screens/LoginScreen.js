import React, { useContext ,useEffect} from 'react';
import { AuthContext } from "../navigation/AppNavigator";

import { View, Text, TextInput, TouchableOpacity, StyleSheet, Image } from 'react-native';
import Icon from 'react-native-vector-icons/FontAwesome';  
import LinearGradient from 'react-native-linear-gradient';

import i18n from '../i18n'; // i18n yapılandırması import edilmeli
import { useTranslation } from 'react-i18next'; 
import * as WebBrowser from 'expo-web-browser';
import * as AuthSession from 'expo-auth-session';

const FB_APP_ID = '1225171182685499'; // <-- buraya kendi App ID'ni yaz
 

// Hatalı native module erişimi engelleniyor
if (WebBrowser?.maybeCompleteAuthSession) {
  WebBrowser.maybeCompleteAuthSession();
}
export default function LoginScreen({ navigation }) {
  const { setIsLoggedIn } = useContext(AuthContext);
  const { t, i18n } = useTranslation();

  const handleLanguageChange = () => {
    const newLang = i18n.language === 'tr' ? 'en' : 'tr';
    i18n.changeLanguage(newLang);
  };
 
    const discovery = {
    authorizationEndpoint: 'https://www.facebook.com/v12.0/dialog/oauth',
    tokenEndpoint: 'https://graph.facebook.com/v12.0/oauth/access_token',
  };


 
useEffect(() => {
    if (response?.type === 'success') {
      const { access_token } = response.params;
      fetch(`https://graph.facebook.com/me?access_token=${access_token}&fields=id,name,email`)
        .then(res => res.json())
        .then(user => {
          Alert.alert('Giriş Başarılı', `Hoş geldin, ${user.name}`);
          console.log('Facebook kullanıcı bilgisi:', user);
          // İstersen backend’e token gönder
        })
        .catch(error => {
          console.error('Kullanıcı bilgisi alınamadı:', error);
        });
    }
  }, [response]);
  return (
    <LinearGradient colors={['#090979', '#00D4FF', '#020024']} style={styles.container}>
      <TouchableOpacity onPress={handleLanguageChange} style={styles.languageSwitcher}>
        <Text style={styles.langText}>
          {i18n.language === 'tr' ? '🇬🇧 EN' : '🇹🇷 TR'}
        </Text>
      </TouchableOpacity>
      <Text style={styles.title}>{t("welcome")}</Text>

      <TextInput
        style={styles.input}
        placeholder={t("email")}
        keyboardType="email-address"
        autoCapitalize="none"
      />
      <TextInput
        style={styles.input}
        placeholder={t("password")}
        secureTextEntry
      />

      <TouchableOpacity style={styles.loginButton}>
        <Text style={styles.loginText}>{t("login")}</Text>
      </TouchableOpacity>

      <Text style={styles.or}>{t("Or")}</Text>

      <View style={styles.socialContainer}>
        <TouchableOpacity style={styles.socialButton}  onPress={() => promptAsync()} >
          <Icon name="facebook" size={20} color="white" />
          <Text style={styles.socialText}>{t("loginWithFacebook")}</Text>
        </TouchableOpacity>

        <TouchableOpacity style={[styles.socialButton, { backgroundColor: '#DB4437' }]}>
          <Icon name="google" size={20} color="white" />
          <Text style={styles.socialText}> {t("loginWithGoogle")}</Text>
        </TouchableOpacity>
      </View>

      <TouchableOpacity>
        <Text style={styles.forgotPassword}>{t("forgotPassword")}</Text>
      </TouchableOpacity>
    </LinearGradient>
  );
}
const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f6fa',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 20,
  },
  logo: {
    width: 100,
    height: 100,
    marginBottom: 30,
  },
  title: {
    fontSize: 24,
    marginBottom: 20,
    fontWeight: '600',
    color: '#2f3640',
  },
  input: {
    width: '100%',
    height: 50,
    backgroundColor: '#dcdde1',
    borderRadius: 8,
    paddingHorizontal: 15,
    marginVertical: 8,
  },
  loginButton: {
    backgroundColor: '#0984e3',
    paddingVertical: 14,
    borderRadius: 8,
    width: '100%',
    marginTop: 10,
    alignItems: 'center',
  },
  loginText: {
    color: 'white',
    fontSize: 16,
    fontWeight: '600',
  },
  or: {
    marginVertical: 15,
    color: '#636e72',
  },
  socialContainer: {
    width: '100%',
    gap: 10,
  },
  socialButton: {
    backgroundColor: '#3b5998',
    paddingVertical: 12,
    borderRadius: 8,
    flexDirection: 'row',
    justifyContent: 'center',
    alignItems: 'center',
  },
  socialText: {
    color: 'white',
    fontSize: 15,
    marginLeft: 8,
  },
  forgotPassword: {
    marginTop: 15,
    color: '#636e72',
    textDecorationLine: 'underline',
  },
   languageSwitcher: {
    position: 'absolute', top: 40, right: 20,
    backgroundColor: '#dfe6e9', paddingVertical: 6,
    paddingHorizontal: 12, borderRadius: 20,
  },
  langText: { fontSize: 14, fontWeight: '600' },
});