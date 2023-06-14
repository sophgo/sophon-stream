from flask import Flask, render_template, Response, request, json
import cv2
import asyncio
import websockets
import base64
from threading import Thread

class VideoCamera(object):
    def __init__(self):
        self.video = None
    def __del__(self):
        if self.video:
            self.video.release()
    def get_frame(self):
        success, image = self.video.read()
        return image
    
app = Flask(__name__)

@app.route('/')   
def index(): #show index page
    return render_template('index.html') 

async def sendImg(websocket, path): #decode and send images to websocket clients
    global indexnum
    camera=video_devices[indexnum-1]
    while True:
            frame = camera.get_frame()
            img_encode = cv2.imencode('.jpg', frame)[1]
            img = img_encode.tobytes()
            s = base64.b64encode(img).decode()
            base64img='data:image/jpeg;base64,{}'.format(s)
            await websocket.send(base64img)

async def mainfunc(port,hostip):     # start a websocket server
    async with websockets.serve(sendImg, hostip, port):
        print("runing ",port)
        await asyncio.Future()

class Compute(Thread):
    def __init__(self, portnum, hostip):
        Thread.__init__(self)
        self.portnum = portnum
        self.hostip = hostip

    def run(self):
        asyncio.run(mainfunc(8764+self.portnum,self.hostip))

@app.route('/pushstream', methods=['POST']) 
def pushstream(): #get post request and start websocket server
    global portnum
    global indexnum
    hostip=request.host
    if indexnum>=16:
        data={"error":"out of Road"}
        response = app.response_class(
        response=json.dumps(data),
        status=500,
        mimetype='application/json'
    ) 
        return response
    data = request.get_json()
    url = data['url']
    video_devices[indexnum].video=cv2.VideoCapture(url)
    if not video_devices[indexnum].video.isOpened():
        video_devices[indexnum].video=None
        data={"error":"can not open rtsp"}
        response = app.response_class(
        response=json.dumps(data),
        status=500,
        mimetype='application/json'
    )
        return response
    portnum+=1
    indexnum+=1
    thread_a =Compute(portnum,hostip.split(':')[0])
    thread_a.start()
    response = app.response_class(status=200)
    return response
     
if __name__ == '__main__':
    video_devices=[]
    indexnum=0
    portnum=0
    for i in range(16):
        video_devices.append(VideoCamera())
    app.run(host='0.0.0.0', debug=True, port=8000)
