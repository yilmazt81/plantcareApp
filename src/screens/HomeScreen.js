import React,{useState} from 'react';
import { View, Text, StyleSheet,ScrollView  } from 'react-native';
import { useNavigation } from '@react-navigation/native';

import { AuthContext } from '../navigation/AppNavigator';
import PlantMoistureCard from '../companent/PlantMoistureCard'
 



const HomeScreen = ({ navigation }) => {

      const [deviceList, setDeviceList] = useState([]);
      const [loading, setLoading] = useState(true);
    // const { setUserToken } = useContext(AuthContext);

    return (
           <ScrollView contentContainerStyle={{ padding: 20 }}>
            <PlantMoistureCard plantName='Papatya' airHumidity={90} temperature={20} soilMoistureLevel={"normal"}></PlantMoistureCard>

            <PlantMoistureCard
                plantName="Sukulent"
                soilMoistureLevel="kuru"
                airHumidity={48}
                temperature={40} />
            
            <PlantMoistureCard
                plantName="Orkide"
                soilMoistureLevel="nemli"
                airHumidity={70}
                temperature={25}/> 
        </ScrollView>
    );
};


export default HomeScreen;