//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

import React, { useEffect, useState, useRef, useContext } from 'react';
import ConfigContext from '../DisplayLayer/DisplayLayer';
import MenuItem from '@material-ui/core/MenuItem';
import Select from '@material-ui/core/Select';
import './VideoDisplay.css'

const serverIpAddress = window.location.hostname;
const serverip = 'http://' + serverIpAddress + ':8000'

const VideoDisplay = () => {
  const canvasRef = useRef(null);
  const [selectedChannel, setSelectedChannel] = useState('0');
  const [channelOptions, setChannelOptions] = useState([]);
  const { selectedPipeline } = useContext(ConfigContext);
  const [isRunning, setIsRunning] = useState(false);
  const [isFirstRender, setIsFirstRender] = useState(true);

  let img = new Image();
  const fps = 25;

  const drawCanvas = () => {
    setTimeout(() => {
      img.onload = () => {
        const canvas = canvasRef.current;
        let canvasCtx = canvas.getContext('2d');
        const { width, height } = canvas;
        canvasCtx.drawImage(img, 0, 0, width, height);
      };
    }, 1000 / fps);
  };

  const resizeCanvas = () => {
    const canvasContainer = document.getElementById('canvasContainer')
    const canvas = canvasRef.current;
    canvas.width = canvasContainer.clientWidth
    canvas.height = canvasContainer.clientHeight
    drawCanvas();
  };

  const handleChannel = event => {
    setSelectedChannel(event.target.value);
  };

  useEffect(() => {
    // 初始化时设置 Canvas 大小
    resizeCanvas();

    // 监听 resize 事件，窗口大小变化时重新设置 Canvas 大小
    window.addEventListener('resize', resizeCanvas);

    // 组件卸载时移除事件监听
    return () => {
      window.removeEventListener('resize', resizeCanvas);
    };
  }, []);

  useEffect(() => {
    if (isFirstRender) {setIsFirstRender(false); return;}

    let port = 9000 + parseInt(selectedChannel);
    const ws = new WebSocket('ws://' + serverIpAddress + `:${port}`);
    ws.onerror = (e) => {
      window.alert('Websocket connect error in this channel, do not switch channel too quickly, please try it latter!');
    }

    ws.addEventListener('open', () => {
      console.log('server connected!');
      ws.send("hello server!");
    });

    ws.addEventListener('message', ({ data }) => {
      img.src = 'data:image/jpeg;base64,' + data;
    });

    drawCanvas();
    return () => {
      ws.close();
    };
  }, [selectedChannel]);

  const handleStartRequest = () => {
    if (isRunning) window.alert('Pipeline is already running, please stop it first!');
    if (selectedPipeline === 'a') window.alert('Please select a pipeline first!')
    setIsRunning(true);
    const startjson = {
      "is_running": true
    };
    fetch(serverip + `/pipelines/${selectedPipeline}`)
      .then(response => response.json())
      .then(result => {
        result.data.map((item) => {
          item.json_list.map((jsoninfo) => {
            if (jsoninfo.json_name.includes('demo.json')) {
              fetch(serverip + '/jsons/' + jsoninfo.json_id)
                .then(response => response.json())
                .then(data => {
                  setChannelOptions(data.data.json_content.channels);
                  fetch(serverip + `/pipelines/${selectedPipeline}`, {
                    method: 'PATCH', // 使用PATCH请求
                    headers: {
                      'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(startjson), // 要发送的数据对象
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
                })
                .catch(error => {
                  console.error(error);
                });
            }
          });
        });
      })
      .catch(error => {
        console.error('Error:setFileContent(jsonString)', error);
      });
  };

  const handleStopRequest = () => {
    if (!isRunning) window.alert('Pipeline is not running, please start it first!');
    setIsRunning(false);
    const stopjson = {
      "is_running": false
    };
    fetch(serverip + `/pipelines/${selectedPipeline}`, {
      method: 'PATCH', // 使用PATCH请求
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(stopjson), // 要发送的数据对象
    })
      .then(response => response.json())
      .then(result => {
        // 处理请求成功的响应数据
        console.log(result);
      })
      .catch(error => {
        // 处理请求失败的情况
        console.error(error);
      });
  };

  console.log('selectedPipeline: ', selectedPipeline);
  return (
    <div className='VideoDisplay'>
      <button className='StartButton' type="button" variant="contained" onClick={handleStartRequest}>
        start pipeline
      </button>
      <button className='StopButton' type="button" variant="contained" onClick={handleStopRequest}>
        stop pipeline
      </button>
      <Select className='ChannelSelector' disabled={!isRunning} value={selectedChannel} onChange={handleChannel}>
        <MenuItem value='0'>Select channel</MenuItem>
        {channelOptions.map((item) => (
          <MenuItem key={item.channel_id} value={item.channel_id}>
            {item.channel_id}
          </MenuItem>
        ))}
      </Select>
      <div className='CanvasContanier' id='canvasContainer'>
        <canvas ref={canvasRef} ></canvas>
      </ div>
    </div>
  )
}

export default VideoDisplay