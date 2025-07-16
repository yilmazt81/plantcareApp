// StatusCard.js

import React from 'react';
import { View, Text, StyleSheet } from 'react-native';
import Icon from 'react-native-vector-icons/MaterialCommunityIcons';
import { AnimatedCircularProgress } from 'react-native-circular-progress';
import {getTemperatureColor} from './iconfunctions';

export default function StatusCard({ icon, temperature, airHumidity, t }) {
  return (
    <View style={styles.card}>
      {/* Sol Kolon */}
      <View style={styles.leftColumn}>
        <Icon name={icon?.name} size={32} color={icon?.color} />
        <Text style={[styles.statusLabel, { color: icon?.color }]}>{t(icon?.label)} </Text>
      </View>

      {/* Orta Kolon */}
      <View style={styles.middleColumn}>
        <Icon name="thermometer" size={28} color={getTemperatureColor(temperature)} />
        <Text style={[styles.temperatureText, { color: getTemperatureColor(temperature) }]}>
          {temperature}°C
        </Text>
      </View>

      {/* Sağ Kolon */}
      <View style={styles.rightColumn}>
        <Text style={styles.gaugeLabel}>{t("AirHumidity")}</Text>
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
}

const styles = StyleSheet.create({
  card: {
    flexDirection: 'row',
    backgroundColor: '#fff',
    borderRadius: 12,
    padding: 16,
    justifyContent: 'space-between',
    alignItems: 'center',
    elevation: 3,
    marginBottom: 12,
  },
  leftColumn: {
    alignItems: 'center',
    flex: 1,
  },
  middleColumn: {
    alignItems: 'center',
    flex: 1,
  },
  rightColumn: {
    alignItems: 'center',
    flex: 1,
  },
  statusLabel: {
    marginTop: 6,
    fontWeight: '600',
  },
  temperatureText: {
    marginTop: 6,
    fontSize: 16,
    fontWeight: 'bold',
  },
  gaugeLabel: {
    marginBottom: 4,
    fontWeight: '600',
  },
  percentageText: {
    fontSize: 14,
    fontWeight: 'bold',
  }
});
