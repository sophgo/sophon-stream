import React from 'react';
import MyComponent from './MyComponent';
const App = () => {
  return (
    <div >
      <div style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '10vh', backgroundColor: 'blue' }}>
      <h1 style={{ color: 'white' }}>Sophon Stream</h1>
      </div>
      <MyComponent />
    </div>
    
  );
};

export default App;