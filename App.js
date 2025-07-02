 
import React, { useState } from 'react';
import AppNavigator, { AuthContext } from './src/navigation/AppNavigator';
 

export default function App() {
  const [isLoggedIn, setIsLoggedIn] = useState(false);

  return (
    <AuthContext.Provider value={{ isLoggedIn, setIsLoggedIn }}>
      <AppNavigator />
    </AuthContext.Provider>
  );
}