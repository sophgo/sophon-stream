//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

import React, { useState, useEffect, useContext } from 'react';
import MenuItem from '@material-ui/core/MenuItem';
import Select from '@material-ui/core/Select';
import ConfigContext from '../DisplayLayer/DisplayLayer';
import './ConfigPreview.css'

const serverIpAddress = window.location.hostname;
const serverip = 'http://' + serverIpAddress + ':8000' //config the ip of backend

const ConfigPreview = () => {
  const [data, setData] = useState([]);
  const [jsonList, setjsonList] = useState([]);
  const [selectedJson, setSelectedJson] = useState('a');
  const [jsonData, setJsonData] = useState('');
  const { selectedPipeline, setSelectedPipeline } = useContext(ConfigContext);

  const handlePipeline = event => {
    setSelectedPipeline(event.target.value);
    data.map((item) => {
      if (item.pipeline_id === event.target.value) {
        setjsonList(item.json_list)
      }
    });
  };

  const handleJson = event => {
    setSelectedJson(event.target.value);
  };

  const handleJsonChange = (event) => {
    setJsonData(event.target.value);
  };

  const handleSubmit = (event) => {
    try {
      const jsonObject = JSON.parse(jsonData);
      event.preventDefault();
      fetch(serverip + `/jsons/${selectedJson}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(jsonObject), // Send the parsed JSON object as the request body
      })
        .then(response => response.json())
        .then(result => {
          // Handle the successful response data
          console.log(result);
          if (result.status === 'error') {
            alert(result.message)
          }
        })
        .catch(error => {
          // Handle request failure
          console.error('Error:', error);
          alert('Request failed: ' + error.message); // Show an alert with the failure message
        });
    } catch (error) {
      // Handle JSON parsing error
      console.error('Invalid JSON:', error);
      alert('Invalid JSON: ' + error.message); // Show an alert with the parsing failure message
    }
  };

  useEffect(() => {
    fetch(serverip + '/pipelines')
      .then(response => response.json())
      .then(result => {
        setData(result.data);
      })
      .catch(error => {
        console.error('Error:setFileContent(jsonString)', error);
      });
  }, []);

  useEffect(() => {
    if (selectedJson !== 'a') {
      console.log(selectedJson);
      fetch(serverip + `/jsons/${selectedJson}`)
        .then(response => response.json())
        .then(data => {
          const jsonString = JSON.stringify(data.data.json_content, null, 2);
          setJsonData(jsonString);
        })
        .catch(error => {
          // Handle any errors that occurred during the request
          console.error(error);
        });
    }
  }, [selectedJson]);

  return (
    <div className='ConfigPreview'>
      <Select className='Select' value={selectedPipeline} onChange={handlePipeline}>
        <MenuItem value='a' >Select pipline</MenuItem>
        {data.map((item) => (
          <MenuItem key={item.pipeline_id} value={item.pipeline_id}>
            {item.pipeline_name}
          </MenuItem>
        ))}
      </Select>
      <Select className='Select' value={selectedJson} onChange={handleJson}>
        <MenuItem value='a'>Select json</MenuItem>
        {jsonList.map((item) => (
          <MenuItem key={item.json_id} value={item.json_id}>
            {item.json_name}
          </MenuItem>
        ))}
      </Select>
      <textarea className='Text' value={jsonData} onChange={handleJsonChange} rows={20} />
      <button className='SaveButton' type="submit" variant="contained" color="primary" onClick={handleSubmit}> Save </button>
    </div>
  )
}

export default ConfigPreview