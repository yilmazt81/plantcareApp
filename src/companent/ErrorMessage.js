import React from 'react';
import { Text, StyleSheet } from 'react-native';

const ErrorMessage = ({ message }) => {
  if (!message) return null;

  return <Text style={styles.error}>{message}</Text>;
};

const styles = StyleSheet.create({
  error: {
    color: '#D32F2F',
    backgroundColor: '#FDECEA',
    padding: 10,
    borderRadius: 8,
    marginVertical: 10,
    textAlign: 'center',
    fontWeight: '600',
  },
});

export default ErrorMessage;
