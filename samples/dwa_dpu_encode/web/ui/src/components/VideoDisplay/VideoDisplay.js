import React, { useEffect, useState, useRef, useContext } from 'react';
import MenuItem from '@material-ui/core/MenuItem';
import Select from '@material-ui/core/Select';
import './VideoDisplay.css'


const VideoDisplay = () => {

  const [displayType, setDisplayType] = useState(0)
  const canvasRef = useRef(null);


  let img = new Image();
  const fps = 25;

  const drawCanvas = () => {
    setTimeout(() => {
      img.onload = () => {
        const canvas = canvasRef.current;
        const canvasCtx = canvas.getContext('2d');
        const canvasWidth = canvas.width;
        const canvasHeight = canvas.height;
  
        // 清除canvas内容
        canvasCtx.clearRect(0, 0, canvasWidth, canvasHeight);
  
        // 计算最适合的缩放比例
        const scale = Math.min(canvasWidth / img.width, canvasHeight / img.height);
  
        // 计算绘制图像的尺寸
        const imgDrawWidth = img.width * scale;
        const imgDrawHeight = img.height * scale;
  
        // 计算绘制图像的位置以使其居中
        const imgDrawX = (canvasWidth - imgDrawWidth) / 2;
        const imgDrawY = (canvasHeight - imgDrawHeight) / 2;
  
        // 绘制图像
        canvasCtx.drawImage(img, imgDrawX, imgDrawY, imgDrawWidth, imgDrawHeight);
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


  const serverIp = window.location.hostname;
  const serverIpWithPort = 'http://' + serverIp + ':8000'

  useEffect(() => {
    let port = 9001;
    const ws = new WebSocket('ws://' + serverIp + `:${port}`);
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
  }, []);


  const handleDisplayType = (event) => {
    setDisplayType(event.target.value)
    const request_json  = {
      "type": event.target.value 
    }
    fetch(serverIpWithPort+`/display-type-dpu`, {
      method: 'PUT', 
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(request_json), 
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
    <div className='VideoDisplay'>

    <div>
      <label>码流选择:</label>

      <Select className='DisplayTypeSelector' value={displayType} onChange={handleDisplayType}>
      <MenuItem value={0}> 原图+深度图 </MenuItem>
      <MenuItem value={1}> DWA+深度图 </MenuItem>
      <MenuItem value={2}> 仅深度图 </MenuItem>
      </Select>
    </div>

      <div className='CanvasContainer' id='canvasContainer'>
        <canvas ref={canvasRef} ></canvas>
      </ div>
    </div>
  )
}

export default VideoDisplay