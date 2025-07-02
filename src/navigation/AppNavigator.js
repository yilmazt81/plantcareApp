import React, { useState, createContext, useContext } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createDrawerNavigator } from '@react-navigation/drawer';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { createNativeStackNavigator } from '@react-navigation/native-stack';
import Icon from 'react-native-vector-icons/Ionicons';


import LoginScreen from '../screens/LoginScreen';
import RegisterScreen from '../screens/RegisterScreen';
import HomeScreen from '../screens/HomeScreen';
import ProfileScreen from '../screens/ProfileScreen';
import SettingsScreen from '../screens/SettingsScreen'; // yeni ekran
import BarcodeScannerScreen from '../screens/BarcodeScannerScreen'; // yeni ekran

const AuthStack = createNativeStackNavigator();
const Tab = createBottomTabNavigator();
const Drawer = createDrawerNavigator();
const Stack = createNativeStackNavigator();
export const AuthContext = React.createContext();

const AuthStackScreen = () => (
  <AuthStack.Navigator>
    <AuthStack.Screen name="Login" component={LoginScreen} options={{headerShown:false}} />
    <AuthStack.Screen name="Register" component={RegisterScreen} />
  </AuthStack.Navigator>
);
const DashboardStack = () => (
  <Stack.Navigator>
    <Stack.Screen
      name="DashboardTabs"
      component={TabNavigator}
      options={({ navigation }) => ({
        title: 'Dashboard',
        // Sol üstte hamburger menü
        headerLeft: () => (
          <Icon
            name="menu-outline"
            size={25}
            color="black"
            style={{ marginLeft: 20 }}
            onPress={() => navigation.toggleDrawer()}
          />
        ),
        // Sağ üstte barkod butonu
        headerRight: () => (
          <Icon
            name="barcode-outline"
            size={25}
            color="black"
            style={{ marginRight: 15 }}
            onPress={() => navigation.navigate('BarcodeScanner')}
          />
        ),
      })}
    />
    <Stack.Screen name="BarcodeScanner" component={BarcodeScannerScreen} />
  </Stack.Navigator>
);

const TabNavigator = () => (
  <Tab.Navigator screenOptions={{ headerShown: false }}>
    <Tab.Screen
      name="Home"
      component={HomeScreen}
      options={{
        tabBarIcon: ({ color, size }) => (
          <Icon name="home-outline" size={size} color={color} />
        ),
      }}
    />
    <Tab.Screen
      name="Profile"
      component={ProfileScreen}
      options={{
        tabBarIcon: ({ color, size }) => (
          <Icon name="person-outline" size={size} color={color} />
        ),
      }}
    />
  </Tab.Navigator>
);
const DrawerNavigator = () => (
  <Drawer.Navigator screenOptions={{ headerShown: false }}>
    <Drawer.Screen
      name="Dashboard"
      component={DashboardStack}
      options={{
        drawerIcon: ({ color, size }) => (
          <Icon name="home-outline" size={size} color={color} />
        ),
      }}
    />
    <Drawer.Screen
      name="Settings"
      component={SettingsScreen}
      options={{
        drawerIcon: ({ color, size }) => (
          <Icon name="settings-outline" size={size} color={color} />
        ),
      }}
    />
  </Drawer.Navigator>
);

export default function AppNavigator() {
  const { isLoggedIn } = useContext(AuthContext);

  return (
    <NavigationContainer>
      {isLoggedIn ? <DrawerNavigator /> : <AuthStackScreen />}
    </NavigationContainer>
  );
}
