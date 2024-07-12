<template>
    <div class="canvas-container">
        <!-- 绘制画布 -->
        <canvas v-for="n in number" :key="n" ref="canvas" :width="width" :height="height" class="my-canvas"></canvas>
    </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue';

// 设置画布大小
const rate = 6
const width = 1920 / rate;
const height = 1080 / rate;

// 创建响应式引用
const number = 16;     // 路数
const frameRate = 25; // 目标帧率
let timer = null;

const canvas = ref([]);
const context = ref([]);
const socket = ref([]);
const queues = ref([])
for (let i = 0; i < number; i++) {
    queues.value.push([])
}
const urls_tmp = ['ws://172.26.13.21:9700/', 'ws://172.26.13.21:9800/', 'ws://172.26.13.21:9900/', 'ws://172.26.13.21:10000/']
const urls = []
for (let i = 0; i < number; i++) {
    urls.push(urls_tmp[i % 4])
}

const messageCountPerSecond = ref(0);
let messageCount = 0; // 用于累计接收的消息数量

// 发送消息的函数
function sendMessage() {
    if (socket.value && socket.value.readyState === WebSocket.OPEN) {
        // 发送文本消息
        socket.value.send('Hello, server!');
        // 如果要发送JSON，可以这样做：
        // socket.value.send(JSON.stringify({ message: 'Hello, server!' }));
    } else {
        console.log('WebSocket is not connected.');
    }
}

function play() {
    for (let i = 0; i < number; i++) {
        if (queues.value[i].length > 0) {
            const data = queues.value[i].shift();
            const image = new Image();
            let boxs = data["mDetectedObjectMetadatas"];
            let frame = data["mFrame"];
            image.onload = () => {
                // 清除画布
                context.value[i].clearRect(0, 0, width, height);
                context.value[i].drawImage(image, 0, 0, width, height);
                if (boxs) {
                    drawRedBox(i, boxs);
                }

            }
            image.src = 'data:image/jpeg;base64,' + frame["mSpData"];
        }
    }
}

// 绘制红色方框的函数
function drawRedBox(i, boxs) {
    // 方框的尺寸
    boxs.forEach(box => {
        let mbox = box["mBox"];
        const boxWidth = mbox["mWidth"] / rate;
        const boxHeight = mbox["mHeight"] / rate;
        const x = mbox["mX"] / rate;
        const y = mbox["mY"] / rate;
        context.value[i].beginPath();
        context.value[i].strokeStyle = 'red';
        context.value[i].lineWidth = 1.5;
        context.value[i].rect(x, y, boxWidth, boxHeight)
        context.value[i].stroke();

    })

}
// 初始化WebSocket连接并设置事件处理
onMounted(() => {
    canvas.value = canvas.value || document.createElement('canvas');
    canvas.value.forEach(canvas => {
        context.value.push(canvas.getContext('2d'))
    })

    for (let i = 0; i < number; i++) {
        socket.value.push(new WebSocket(urls[i]));
        socket.value[i].onmessage = (event) => {
            messageCount++;
            try {
                const data = JSON.parse(event.data);
                queues.value[i].push(data);
            } catch (error) {
                console.error(error);
                console.log(event.data);
            }
        }
    }


    // 设置定时器以固定帧率播放图片
    timer = setInterval(() => {
        play();
    }, 1000 / frameRate);


    // 每秒更新一次消息计数器
    setInterval(() => {
        messageCountPerSecond.value = messageCount;
        console.log(messageCountPerSecond.value)
        messageCount = 0; // 重置计数器
    }, 1000);


    socket.value.onopen = () => {
        console.log('WebSocket Connected');
    };

    socket.value.onerror = (error) => {
        console.log('WebSocket Error:', error);
    };
});




// 当组件卸载时关闭WebSocket连接
onUnmounted(() => {
    for (let i = 0; i < number; i++) {
        if (socket.value[i]) {
            socket.value[i].close()
        }
    }

    if (timer) {
        clearInterval(timer);
    }
    
});

</script>

<style scoped>
canvas {
    /* 为了确保画布不会有额外的空间或者边框 */
    display: block;
}

.canvas-container {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    /* 每行4个canvas */
    gap: 15px;
    /* canvas之间的间隙 */
}

.my-canvas {
    /* width: 100px; 或根据需要设置宽度 */
    /* height: 100px; 或根据需要设置高度 */
    background-color: #f0f0f0;
    /* 背景色，便于区分 */
}
</style>
