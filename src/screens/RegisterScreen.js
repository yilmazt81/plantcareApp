import React, { useState } from 'react';
import { View, Text, TextInput, Button, StyleSheet } from 'react-native';
 
import LinearGradient from 'react-native-linear-gradient';


import auth from '@react-native-firebase/auth';
import firestore from '@react-native-firebase/firestore';

const RegisterScreen = ({ navigation }) => {
    const [fullName, setFullName] = useState('');
    const [phone, setPhone] = useState('');
    const [email, setEmail] = useState('');
    const [password, setPassword] = useState('');
    const handleRegister = async () => {
        try {
            // Firebase Auth ile kullanıcı oluştur
            
            const userCredential = await auth().createUserWithEmailAndPassword(email, password);
            const uid = userCredential.user.uid;

            // Firestore'a kullanıcı bilgilerini kaydet
            await firestore().collection('users').doc(uid).set({
                fullName: fullName,
                phone: phone,
                email: email,
                createdAt: firestore.FieldValue.serverTimestamp(),
            });

            alert('Kayıt başarılı!');
            navigation.navigate("Login"); // Kayıt başarılı olduktan sonra Login ekranına yönlendir
        } catch (error) {
            alert(error.message);
            console.log('Registration error:', error);
        }
    };

 

    return (
        <LinearGradient colors={['#090979', '#00D4FF', '#020024']} style={styles.container}>
            <Text style={styles.title}>Register</Text>
             <TextInput
                style={styles.input}
                placeholder="Full Name"
                value={fullName}
                onChangeText={setFullName}
                autoCapitalize="none"
            />
           
             <TextInput
                style={styles.input}
                placeholder="Phone Number"
                value={phone}
                onChangeText={setPhone}
                autoCapitalize="none"
            />
            <TextInput
                style={styles.input}
                placeholder="Email"
                value={email}
                onChangeText={setEmail}
                keyboardType="email-address"
                autoCapitalize="none"
            />
            <TextInput
                style={styles.input}
                placeholder="Password"
                value={password}
                onChangeText={setPassword}
                secureTextEntry
            />
            <Button title="Register" onPress={handleRegister} />
        </LinearGradient>
    );
};

const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        padding: 24,
        backgroundColor: '#fff',
    },
    title: {
        fontSize: 28,
        fontWeight: 'bold',
        marginBottom: 32,
        textAlign: 'center',
    },
    input: {
        height: 48,
        borderColor: '#ccc',
        borderWidth: 1,
        borderRadius: 8,
        marginBottom: 16,
        paddingHorizontal: 12,
        fontSize: 16,
    },
});

export default RegisterScreen;