import React, { useEffect, useState} from 'react';
import { styled } from '@mui/system';
import { Select, MenuItem, Button, TextareaAutosize } from '@mui/material';
const serverip='http://localhost:8000' //config the ip of backend
const Container = styled('div')({
    display: 'flex',
    justifyContent: 'space-between',
  });
  const MediaContainer = styled('div')({
    display: 'flex',
    justifyContent: 'space-between',
  });
  const CanvasContainer = styled('div')({
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    marginLeft: '3vw',
    marginRight: '3vw',
    marginTop: '5vh',
  });
  const StyledSelectl = styled(Select)({
    minWidth: '16vw', // Adjust the width of the Select components as desired
    marginLeft: '3vw',
    marginTop: '1vh',
  });
  const StyledSelectr = styled(Select)({
    minWidth: '16vw', // Adjust the width of the Select components as desired
    marginRight: '3vw',
    marginTop: '1vh',
  });
  const StyledButton = styled(Button)({
    minWidth: '15vw',
    minHeight: '2vw',
    marginRight: '3vw',
    marginLeft: '3vw',
    marginTop: '1vh',
    backgroundColor: '#007bff', // Change the background color to blue (#007bff)
    color: '#ffffff', // Change the text color to white
    '&:hover': {
      backgroundColor: '#0056b3', // Change the background color on hover
    },
    borderRadius: '25px',
  });
  const Textarea = styled('textarea')({
    width: '35vw', // Set the desired width for the textarea
    height: '50vh', // Set the desired height for the textarea
    marginTop: '5vh',
    marginLeft: '3vw',
    resize: 'vertical', // Allow vertical resizing of the textarea
    overflowY: 'auto', // Enable vertical scrolling when content overflows
  });

function MyComponent() {
    const [data, setData] = useState([]);
  const [selectedOption, setSelectedOption] = useState('a');
  const [selectedOption2, setSelectedOption2] = useState('a');
  const [selectedOption3, setSelectedOption3] = useState('0');
  const [options, setOptions] = useState([]);
  const [channelOptions, setChannelOptions] = useState([]);
  const [jsonData, setJsonData] = useState('');
  useEffect(() => {
    fetch(serverip+'/pipelines')
      .then(response => response.json())
      .then(result => {
        setData(result.data);
      })
      .catch(error => {
        console.error('Error:setFileContent(jsonString)', error);
      });
  }, []);

  useEffect(() => {
    let port=9000+parseInt(selectedOption3);
    const ws = new WebSocket(`ws://127.0.0.1:${port}`);

    ws.addEventListener('open', () => {
      console.log('server connected!');
      ws.send("hello server!");
    });

    let img = new Image();
    let fps = 25;
    let myCanvas = document.getElementById("myCanvas").getContext('2d');

    ws.addEventListener('message', ({ data }) => {
      img.src = data;
    });

    setTimeout(() => {
      img.onload = () => {
        myCanvas.drawImage(img, 0, 0, 960, 540);
      };
    }, 1000 / fps);

    return () => {
      ws.close();
    };
  }, [selectedOption3]);

  const handleChange = event => {
    setSelectedOption(event.target.value);
    data.map((item) => {
      if(item.pipeline_id===event.target.value){
        setOptions(item.json_list)
      }
    });
  };
  const handleChange2 = event => {
    setSelectedOption2(event.target.value);
  };
  const handleChange3 = event => {
    setSelectedOption3(event.target.value);
  };
  
  useEffect(() => {
    if (selectedOption2 !== 'a') {
      fetch(serverip+'/jsons/${selectedOption2}')
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
  }, [selectedOption2]);

  const handleJsonChange = (event) => {
    setJsonData(event.target.value);
  };

  const handleSubmit = (event) => {
    try {
      const jsonObject = JSON.parse(jsonData);
      event.preventDefault(); 
      fetch(serverip+'/jsons/${selectedOption2}', {
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

  const handlePatchRequest = () => {
    const startjson = {
      "is_running": true
    };
    fetch(serverip+'/pipelines/${selectedOption}', {
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
      });
      if (selectedOption =='4') {
        fetch(serverip+'/jsons/35')
        .then(response => response.json())
        .then(data => {
          // const chnnels = JSON.stringify(data.data.json_content.channels, null, 2);
          setChannelOptions(data.data.json_content.channels);
        })
        .catch(error => {
          // Handle any errors that occurred during the request
          console.error(error);
        });
      }
      if (selectedOption =='5') {
        fetch(serverip+'/jsons/45')
        .then(response => response.json())
        .then(data => {
          // const chnnels = JSON.stringify(data.data.json_content.channels, null, 2);
          setChannelOptions(data.data.json_content.channels);
        })
        .catch(error => {
          // Handle any errors that occurred during the request
          console.error(error);
        });
      }
  };

  const handlePatchRequest2 = () => {
    const stopjson = {
      "is_running": false
    };
    fetch(serverip+'/pipelines/${selectedOption}', {
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
    return(
        <div>
        <Container>
          <div>
            <StyledSelectl value={selectedOption} onChange={handleChange}>
                <MenuItem value='a'>Select the pipline</MenuItem>
                {data.map((item) => (
                <MenuItem key={item.pipeline_id} value={item.pipeline_id}>
                    {item.pipeline_name}
                </MenuItem>
                ))}
            </StyledSelectl>
            <StyledSelectl value={selectedOption2} onChange={handleChange2} >
            <MenuItem value='a'>Select the json file</MenuItem>
            {options.map((option) => (
              <MenuItem key={option.json_id} value={option.json_id}>
                {option.json_name}
              </MenuItem>
            ))}
            </StyledSelectl>
          </div>
          <div>
            <StyledButton type="button" variant="contained" color="primary" onClick={handlePatchRequest}>
            start pipeline
            </StyledButton>
            <StyledButton type="button" variant="contained" color="primary" onClick={handlePatchRequest2}>
            stop pipeline
            </StyledButton>
            <StyledSelectr value={selectedOption3} onChange={handleChange3}>
            <MenuItem value='0'>Select the channel</MenuItem>
            {channelOptions.map((item) => (
              <MenuItem key={item.channel_id} value={item.channel_id}>
                {item.channel_id}
              </MenuItem>
            ))}
            </StyledSelectr>
          </div>
        </Container>
        <MediaContainer>
        <Textarea value={jsonData} onChange={handleJsonChange} rows={20} />
        <CanvasContainer>
        <canvas id="myCanvas" width="960" height="540"></canvas>
        </CanvasContainer>
        </MediaContainer>
        <StyledButton type="submit" variant="contained" color="primary" onClick={handleSubmit}>
            save
        </StyledButton>
        </div>
        
    );
}
export default MyComponent;