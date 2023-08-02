//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

import React, { createContext, useState } from 'react'
import ConfigPreview from '../ConfigPreview/ConfigPreview';
import VideoDisplay from '../VideoDisplay/VideoDisplay';
import './DisplayLayer.css'
const ConfigContext = createContext();

export const DisplayLayer = () => {
  const [selectedPipeline, setSelectedPipeline] = useState('a');

  return (
    <ConfigContext.Provider value={{ selectedPipeline, setSelectedPipeline }}>
      <div className='DisplayLayer'>
        <ConfigPreview />
        <VideoDisplay />
      </div>
    </ConfigContext.Provider>
  )
}

export default ConfigContext