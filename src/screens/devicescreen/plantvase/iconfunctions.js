import React, { useState } from 'react';

export const getMoistureIcon = (level) => {

    switch (level) {
        case 'nemli':
            return { name: 'water', color: '#4fc3f7', label: 'Nemli' };
        case 'normal':
            return { name: 'water-outline', color: '#81c784', label: 'Normal' };
        case 'kuru':
        default:
            return { name: 'water-off', color: '#e57373', label: 'Kuru' };
    }
};

 export  const getSoilMoistureLevel = (soilMoisture) => {
        var soilMoisText = "";
        if (soilMoisture < 30) {
            soilMoisText = 'kuru';
        } else if (soilMoisture < 70) {
            soilMoisText = 'normal';
        } else {
            soilMoisText = 'nemli';
        }

        return soilMoisText;
    };

export const getTemperatureColor = (temperature) => {
    if (temperature <= 0) return '#42a5f5'; // mavi
    if (temperature >= 40) return '#ef5350'; // kırmızı
    if (temperature <= 20) {
        // mavi → yeşil arası geçiş
        const ratio = temperature / 20;
        return interpolateColor('#42a5f5', '#66bb6a', ratio);
    } else {
        // yeşil → kırmızı arası geçiş
        const ratio = (temperature - 20) / 20;
        return interpolateColor('#66bb6a', '#ef5350', ratio);
    }
};

export const interpolateColor = (color1, color2, factor) => {
    const hexToRgb = (hex) =>
        hex.match(/\w\w/g).map((c) => parseInt(c, 16));

    const rgbToHex = (rgb) =>
        '#' +
        rgb
            .map((x) => {
                const hex = Math.round(x).toString(16);
                return hex.length === 1 ? '0' + hex : hex;
            })
            .join('');

    const c1 = hexToRgb(color1);
    const c2 = hexToRgb(color2);

    const result = c1.map((c, i) => c + (c2[i] - c) * factor);
    return rgbToHex(result);
};

 