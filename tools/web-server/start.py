import os
import shutil
import requests
import json
import subprocess
import sys
script_dir = os.path.dirname(os.path.realpath(__file__))


import time
os.chdir(script_dir)
process = subprocess.Popen(['bash', 'start_server.sh',0])
import time
time.sleep(5)

# 指定路径
source_path = script_dir+"/stream_config_back"

if not os.path.exists(source_path):
    os.makedirs(source_path)

# print(source_path)
# 定义请求的 URL
def create(data):
    url = "http://0.0.0.0:8001/task/create"

    # 定义请求头（如果有的话）
    headers = {
        "Content-Type": "application/json"
    }
    print(data)
    # 发送 POST 请求
    response = requests.post(url, json=data, headers=headers)

    # 打印响应内容
    print(response.status_code)
    print(response.text)
def task_list():
    url = "http://0.0.0.0:8001/task/list"

    # 定义请求头（如果有的话）
    headers = {
        "Content-Type": "application/json"
    }
    # print(data)
    # 发送 POST 请求
    data={}
    response = requests.post(url, json=data, headers=headers)

    # 打印响应内容
    print(response.status_code)
    print(response.text)
    return response.text

# 读取包含task_id的txt文件
if  os.path.exists(source_path+"/task_ids.txt"):

    with open(os.path.join(source_path, "task_ids.txt"), 'r') as file:
        task_ids = [line.strip() for line in file]
else:
    task_ids = []
# 遍历每个task_id，将对应文件夹下的文件移动到目标路径
print(source_path)
for task_id in task_ids:
    with open(os.path.join(source_path, str(task_id)+".json"), 'r') as json_file:
        task_id_dict = json.load(json_file)
        create(task_id_dict)

# 执行命令

while 1:
    result=json.loads(task_list())
    print(result)
    print(result["Result"])
    for i in result["Result"]:
        if(i["Status"]==0):
            task_id=i["TaskID"]
            with open(os.path.join(source_path, str(task_id)+".json"), 'r') as json_file:
                task_id_dict = json.load(json_file)
                create(task_id_dict)
                
    time.sleep(10)

process.wait()