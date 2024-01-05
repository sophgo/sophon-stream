
import React, { useEffect, useState, useRef, useContext } from 'react';
import "./ConfigLayer.css"

const ConfigLayer = () => {

  const [configData, setConfigData] = useState(new Map([
    ['DpuMode', ''],
    ['MaskMode', ''],
    ['DispRange', ''],
    ['DccDir', ''],
    ['DepthUnit', ''],
    ['DispStartPos', ''],
    ['Rshift1', ''],
    ['Rshift2', ''],
    ['CaP1', ''],
    ['CaP2', ''],
    ['UniqRatio', ''],
    ['DispShift', ''],
    ['CensusShift', ''],
    ['FxbaseLine', ''],
    ['MaxCount', ''],
    ['MaxT', ''],
  ]));
  const columns = 4;

  const handleInputChange = (e, label) => {
    const updatedConfigData = new Map(configData);
    updatedConfigData.set(label, e.target.value);
    setConfigData(updatedConfigData);
  };


  const generateTable = () => {
    const rows = Math.ceil(configData.size / columns);
    let table = [];

    let dataIndex = 0;
    for (let i = 0; i < rows; i++) {
      let row = [];
      for (let j = 0; j < columns; j++) {
        if (dataIndex < configData.size) {
          const label = Array.from(configData.keys())[dataIndex];
          row.push(
            <td key={j}>
              <label>{label}</label>
              <input
                type="text"
                value={configData.get(label)}
                onChange={(e) => handleInputChange(e, label)}
              />
            </td>
          );
          dataIndex++;
        }
      }
      table.push(<tr key={i}>{row}</tr>);
    }
    return table;
  };



  const serverIp = window.location.hostname;
  const serverIpWithPort = 'http://' + serverIp + ':8000'


  const handleGetConfig = (event) => {
    fetch(serverIpWithPort + `/config`, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    })
      .then(response => response.json())
      .then(result => {
        // 处理请求成功的响应数据
        const updatedMap = new Map();

        Object.entries(result.data).forEach(([key, value]) => {
          updatedMap.set(key, value);
        });
        setConfigData(updatedMap);

      })
      .catch(error => {
        // 处理请求失败的情况
        console.error(error);
        alert('Request failed: ' + error.message); // Show an alert with the failure message
      });
  }

  const handleSetConfig = (event) => {
    fetch(serverIpWithPort + `/config`, {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(Object.fromEntries(configData)),
    })
      .then(response => response.json())
      .then(result => {
        // 处理请求成功的响应数据
        console.log(result);
      })
      .catch(error => {
        // 处理请求失败的情况
        console.error(error);
        alert('Request failed: ' + error.message); // Show an alert with the failure message
      });
  }

  return (
    <div className='ConfigLayer'>
  <div className='buttonsContainer'>
    <button className='configButton' type="button" variant="contained" onClick={handleGetConfig}>
      获取配置
    </button>
    <button className='configButton' type="button" variant="contained" onClick={handleSetConfig}>
      保存配置
    </button>
  </div>
  <table border="1">
    <tbody>{generateTable()}</tbody>
  </table>
</div>

  )
}

export default ConfigLayer