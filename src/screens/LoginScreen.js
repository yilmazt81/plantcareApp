import React, { useContext, useEffect, useState } from 'react';

import { View, Text, TextInput, TouchableOpacity, StyleSheet, Image } from 'react-native';
import Icon from 'react-native-vector-icons/FontAwesome';
import LinearGradient from 'react-native-linear-gradient';

import i18n from '../i18n'; // i18n yapılandırması import edilmeli
import { useTranslation } from 'react-i18next';
import auth from '@react-native-firebase/auth';
import firestore from '@react-native-firebase/firestore';
import { AuthContext } from '../navigation/AppNavigator';
import { LoginManager, AccessToken } from 'react-native-fbsdk-next';
import handleFacebookLogin from '../companent/facebookLogin'; // Facebook login fonksiyonunu import ediyoruz
import ErrorMessage from '../companent/ErrorMessage';

export default function LoginScreen({ navigation }) {

  const { t, i18n } = useTranslation();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const { setUserToken } = useContext(AuthContext);
  const [loginError, setLoginError] = useState(null);

  const handleLogin = () => {
    auth()
      .signInWithEmailAndPassword(email, password)
      .then(userCredential => {
        const uid = userCredential.user.uid;
        setUserToken(uid); // AppNavigator'da kullanıcıyı login etmiş sayıyoruz
      })
      .catch(error => {
        setLoginError(t(error.code));// örnek: Şifre hatalıysa gösterilir
      });
  };

  const handleLanguageChange = () => {
    const newLang = i18n.language === 'tr' ? 'en' : 'tr';
    i18n.changeLanguage(newLang);
  };




  return (
    <LinearGradient colors={['#090979', '#00D4FF', '#020024']} style={styles.container}>
      <TouchableOpacity onPress={handleLanguageChange} style={styles.languageSwitcher}>
        <Text style={styles.langText}>
          {i18n.language === 'tr' ? '🇬🇧 EN' : '🇹🇷 TR'}
        </Text>
      </TouchableOpacity>
      <Image
        source={require('../../assets/Logo.png')}
      />
      <Text style={styles.title}>{t("welcome")}</Text>

      <TextInput
        style={styles.input}
        placeholder={t("email")}
        keyboardType="email-address"
        autoCapitalize="none"
        value={email} onChangeText={setEmail}
      />
      <TextInput
        style={styles.input}
        placeholder={t("password")}
        secureTextEntry
        value={password} onChangeText={setPassword}
      />

      <ErrorMessage message={loginError} />

      <TouchableOpacity style={styles.loginButton} onPress={handleLogin}>
        <Text style={styles.loginText}>{t("login")}</Text>
      </TouchableOpacity>


      <TouchableOpacity style={styles.registerButton} onPress={() => navigation.navigate('Register')}>
        <Text style={styles.loginText}>{t("register")}</Text>
      </TouchableOpacity>
      <Text style={styles.or}>{t("Or")}</Text>

      <View style={styles.socialContainer}>

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
  registerButton: {
    backgroundColor: '#0984e3',
    paddingVertical: 14,
    borderRadius: 8,
    width: '100%',
    marginTop: 10,
    alignItems: 'center',
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
    color: '#FFFBDE',
    textDecorationLine: 'underline',
  },

  loginButton: {
    backgroundColor: '#0984e3',
    paddingVertical: 14,
    borderRadius: 8,
    width: '100%',
    marginTop: 10,
    alignItems: 'center',
  },
  languageSwitcher: {
    position: 'absolute', top: 40, right: 20,
    backgroundColor: '#dfe6e9', paddingVertical: 6,
    paddingHorizontal: 12, borderRadius: 20,
  },
  langText: { fontSize: 14, fontWeight: '600' },
});