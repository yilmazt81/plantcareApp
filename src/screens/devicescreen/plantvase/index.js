import React from 'react';
import { View, Text, StyleSheet } from 'react-native';

const Index = () => {
    return (
        <View style={styles.container}>
            <Text style={styles.title}>Plant Vase</Text>
            {/* Add your component content here */}
        </View>
    );
};

const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#fff',
    },
    title: {
        fontSize: 24,
        fontWeight: 'bold',
    },
});

export default Index;