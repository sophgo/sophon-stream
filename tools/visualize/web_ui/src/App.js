//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

import React from 'react';
import Navigation from './components/Navigation/Navigation';
import { DisplayLayer } from './components/DisplayLayer/DisplayLayer';
import './App.css';

const App = () => {
  return (
    <div >
      <Navigation />
      <DisplayLayer />
    </div>
  );
};

export default App;