import React, { useState } from 'react';
import logo from './logo.svg';
import './App.css';
import VideoDisplay from './components/VideoDisplay/VideoDisplay';
import VideoDisplayBlend from './components/VideoDisplayBlend/VideoDisplay';
import ConfigLayer from './components/ConfigLayer/ConfigLayer';

function App() {
  // 定义一个状态来存储下拉菜单的选择
  const [selectedOption, setSelectedOption] = useState('DPU'); // 默认值为显示两个组件

  // 处理下拉菜单选择变化的函数
  const handleSelectChange = (event) => {
    setSelectedOption(event.target.value);
  };

  return (
    <div className="App">
      {/* 下拉菜单 */}
      <select value={selectedOption} onChange={handleSelectChange}>
        <option value="DPU">DPU</option>
        <option value="BLEND">BLEND</option>
      </select>

      {/* 根据下拉菜单的选择来决定显示哪个组件 */}
      {selectedOption === 'DPU' && (
        <>
          <VideoDisplay />
          <ConfigLayer />
        </>
      )}
      {selectedOption === 'BLEND' && <VideoDisplayBlend />}
    </div>
  );
}

export default App;
