import React, { useState, createContext, useContext, useEffect } from 'react';
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
import WifiSettingsScreen from '../screens/WifiSettingsScreen'; // yeni ekran
import auth from '@react-native-firebase/auth';
import WifiScannerScreen from '../screens/WifiScannerScreen'; // yeni ekran


const AuthStack = createNativeStackNavigator();
const Tab = createBottomTabNavigator();
const Drawer = createDrawerNavigator();
const Stack = createNativeStackNavigator();

export const AuthContext = React.createContext();

const AuthStackScreen = () => (
  <AuthStack.Navigator>
    <AuthStack.Screen name="Login" component={LoginScreen} options={{ headerShown: false }} />
    <AuthStack.Screen name="Register" component={RegisterScreen} />
  </AuthStack.Navigator>
);



const DashboardStack = () => {
  const { setUserToken } = useContext(AuthContext); // 🔑

  const logout = () => {
    auth()
      .signOut()
      .then(() => {
        setUserToken(null);
      });
  };

  return (
    <Stack.Navigator>
      <Stack.Screen
        name="DashboardTabs"
        component={TabNavigator}
        options={({ navigation }) => ({
          title: 'Dashboard',
          headerLeft: () => (
            <Icon
              name="menu-outline"
              size={25}
              color="black"
              style={{ marginLeft: 20 }}
              onPress={() => navigation.toggleDrawer()}
            />
          ),
          headerRight: () => (
            <>
              <Icon
                name="barcode-outline"
                size={25}
                color="black"
                style={{ marginRight: 15 }}
                onPress={() => navigation.navigate('BarcodeScanner')}
              />
              <Icon
                name="log-out-outline"
                size={25}
                color="black"
                style={{ marginRight: 15 }}
                onPress={logout}
              />
            </>
          ),
        })}
      />
      <Stack.Screen name="BarcodeScanner" component={BarcodeScannerScreen} />
      <Stack.Screen name="WifiSettings" component={WifiSettingsScreen} />
      <Stack.Screen name="WifiScanner" component={WifiScannerScreen} />
    </Stack.Navigator>
  );
};

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


  const [userToken, setUserToken] = useState(null);

  useEffect(() => {


    console.log("LoginScreen:", LoginScreen); // undefined mı?
    console.log("DrawerNavigator:", DrawerNavigator);

    const unsubscribe = auth().onAuthStateChanged(user => {
      if (user) {
        setUserToken(user.uid);
      } else {
        setUserToken(null);
      }
    });

    return unsubscribe;
  }, []);

  return (
    <AuthContext.Provider value={{ setUserToken }}>
      <NavigationContainer>
        {userToken == null ? <AuthStackScreen /> : <DrawerNavigator />}
      </NavigationContainer>
    </AuthContext.Provider>
  );
}
