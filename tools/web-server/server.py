from flask import Flask, request, jsonify
import requests
import argparse
import multiprocessing 
from multiprocessing import Process
import time
import json
import subprocess
import os
import signal
from config_algorithm import *
import os
import glob
import shutil

print(__name__)
app = Flask(__name__)
stream_path=""
current_directory = os.getcwd()
process_pools={}
infos={}
Types={}
port=10001
task_ports={}
algorithms=Algorithms()
ports_json={
      "output": [
                        {
                            "port_id": 0,
                            "is_sink": True,
                            "is_src": False
             }
    ]
}
def build_config(data):
    global port
    # try:
    demo_config_paths,TaskTypes=[],[]
    task_ports[data['TaskID']]=[]
    for i in range(len(data['Algorithm'])):
        algorithm_name=map_type[data['Algorithm'][i]["Type"]]
        config_path=stream_path+'/samples/'+algorithm_name+'/config/'
        engine_group_path=stream_path+'/samples/'+algorithm_name+'/config/engine_group.json'
        with open(engine_group_path, 'r') as file:
        # 使用 json.load 将文件内容转换为字典
            json_data = json.load(file)
        max_element_id=0
        http_push_found = False
        http_string="../"+algorithm_name+"/config/http_push.json"
        filter_found = False
        filter_string="../"+algorithm_name+"/config/filter.json"
        for element in json_data[0]['elements']:
            if element.get('element_config') == filter_string:
                filter_found = True
        if filter_found==False:
            for element in json_data[0]['elements']:
                max_element_id=max(int(element['element_id']),max_element_id)
                if 'ports' in element:
                    ports = element['ports']
                    if 'output' in ports:
                        # 如果存在 output，记录 element_id
                        element_id=element['element_id']
                        # 删除 ports 字段
                        del element['ports']
            filter_element={}
            filter_element["element_id"]=max_element_id+1  
            filter_element["element_config"]=filter_string
            json_data[0]['elements'].append(filter_element)
            filter_connect={
                "src_element_id": element_id,
                "src_port": 0,
                "dst_element_id": max_element_id+1,
                "dst_port": 0
            }
            json_data[0]['connections'].append(filter_connect)
        for element in json_data[0]['elements']:
            if element.get('element_config') == http_string:
                http_push_found = True
        if http_push_found==False:
            http_push_element={}
            http_push_element["element_id"]=max_element_id+2  
            http_push_element["element_config"]=http_string
            http_push_element["ports"]=ports_json
            json_data[0]['elements'].append(http_push_element)
            http_push_connect={
                "src_element_id": max_element_id+1,
                "src_port": 0,
                "dst_element_id": max_element_id+2,
                "dst_port": 0
            }
            json_data[0]['connections'].append(http_push_connect)

        
        with open(engine_group_path, 'w') as file:
            json.dump(json_data, file, indent=2)
        cmd=["cp","config/http_push.json",config_path]
        cp_process = subprocess.Popen(cmd, shell=False)    
        erro=cp_process.wait()
        cmd=["cp","config/filter.json",config_path]
        cp_process = subprocess.Popen(cmd, shell=False)    
        erro=cp_process.wait()
        algorithm_build_config=getattr(algorithms,algorithm_name+'_build_config')
        demo_config_path=algorithm_build_config(algorithm_name,stream_path,data,port,i)
        task_ports[data['TaskID']].append(port)
        port+=1
        demo_config_paths.append(demo_config_path)
        TaskTypes.append(data['Algorithm'][i]["Type"])
    return demo_config_paths,data['TaskID'],TaskTypes
    # except:
    #    return "no type",data['TaskID'],data['Algorithm'][0]["Type"]
def build_client(task_id,Type,id,src_id,result_url):
    # import pdb; pdb.set_trace()
    
    # client_app.run(debug=True, host='0.0.0.0', port=8000)
    cmd=["python3","client.py","--task_id="+str(task_id),"--type="+str(Type),"--port="+str(task_ports[task_id][id]),"--id="+str(id),"--src_id="+str(src_id),"--url="+str(result_url)]
    # cmd="python client.py --task_id="+str(task_id)+" --type="+str(Type)

    print(cmd)
    log_path=str(task_id)+"_"+str(id)+"_client.log"
    with open(log_path, "w") as log_file:
        process = subprocess.Popen(cmd, shell=False, stdout=log_file, stderr=subprocess.STDOUT)
    return process

def build_task(demo_config_path,task_id,Type,id,src_id,result_url):
   
    print(demo_config_path)
    print("Worker process started")
    #time.sleep(3)
    stream_run_path=stream_path+"/samples/build"
    client_process = build_client(task_id,Type,id,src_id,result_url)   

    os.chdir(stream_run_path)
    cmd=[stream_run_path+"/main","--demo_config_path="+demo_config_path]
    print(cmd)
    log_path=str(task_id)+"_"+str(id)+"_stream.log"
    with open(log_path, "w") as log_file:
        stream_process = subprocess.Popen(cmd, shell=False, stdout=log_file, stderr=subprocess.STDOUT)    
    os.chdir(current_directory)

    
    process_pools[task_id].append((stream_process,client_process))

    infos[task_id]={}
    Types[task_id].append(Type)

    return 0
def task_status(task_id):
    if(task_id in process_pools.keys()):
        res={}
        res["TaskID"]=task_id
        # print(int(process_pools[task_id][0].poll()==None))
        status=0
        for i in range(len(process_pools[task_id])):
            status=status|int((process_pools[task_id][i][0].poll()==None))
        res["Status"]=status
        return res
    return {"Status":"No task"}
def task_list():
    print(infos)
    print(Types)
    ans=[]
    for key in infos.keys():
        ans.append(task_status(key))
    return ans
def del_task(task_id):
    if(task_status(task_id)["Status"]==1):
        # os.kill(process_pools[tasks_ids[task_id]].pid, signal.SIGINT)
        for i in range(len(process_pools[task_id])):
            process_pools[task_id][i][0].kill()
            process_pools[task_id][i][1].kill()
        del process_pools[task_id]
        del infos[task_id]
        del Types[task_id]
        # process_pools[tasks_ids[task_id]][1].terminate()
        return 0
    else:
        return "No task or task ended"


@app.route('/task/delete', methods=['POST'])
def receive_request2():
    print(request.json)
    ans=del_task(str(request.json['TaskID']))
    if(ans == 0):
        return jsonify({"Code": 0, "Msg": "success"})
    else:
        return jsonify({"Code": -1, "Msg": str(ans)})
@app.route('/task/query', methods=['POST'])
def receive_request3():
    ans=task_status(str(request.json['TaskID']))
    if(ans['Status']==1):
        return jsonify({"Code": 0, "Msg": "success","Result":ans})
    else:
        return jsonify({"Code": 1, "Msg": "fail"})

@app.route('/task/list', methods=['POST'])
def receive_request66():
    # print(infos)
    return jsonify({"Code": 0, "Msg": "success","Result":task_list()})

@app.route('/task/create', methods=['POST'])
def receive_request():
    # 获取来自客户端的 JSON 数据
    demo_config_paths,task_id,Types_= build_config(request.json)
    # if(demo_config_path=="no type"):
    #     return jsonify({"Code": -1, "Msg": "no type"})
    print(request.json["Reporting"]["ReportUrlList"])
    stream_run_path=stream_path+"/samples/build"
    # 在这里处理接收到的数据
    process_pools[task_id]=[]
    Types[task_id]=[]
    if(task_status(task_id)["Status"]==1):
        return jsonify({"Code": -1, "Msg": "task is running"})
    for i in range(len(demo_config_paths)):
        
        build_task(demo_config_paths[i],task_id,Types_[i],i,request.json["InputSrc"]["SrcID"],request.json["Reporting"]["ReportUrlList"])

    return jsonify({"Code": 0, "Msg": "success"})
    
def argsparser():
    parser = argparse.ArgumentParser(prog=__file__)
    parser.add_argument('--stream_path', type=str, default='/home/hyc/data/sophon-stream', help='path of stream')
    args = parser.parse_args()
    return args
if __name__ == '__main__':
    args = argsparser()
    stream_path=args.stream_path
    # 获取当前目录下所有的.log文件
    log_files = glob.glob('*.log')
    # 遍历文件列表，逐个删除
    for log_file in log_files:
        try:
            os.remove(log_file)
            print(f"Deleted: {log_file}")
        except OSError as e:
            print(f"Error: {log_file} : {e.strerror}")
            
    log_files = glob.glob(stream_path+'/samples/build/*.log')
    # 遍历文件列表，逐个删除
    for log_file in log_files:
        try:
            os.remove(log_file)
            print(f"Deleted: {log_file}")
        except OSError as e:
            print(f"Error: {log_file} : {e.strerror}")
    if os.path.isdir('results'):
        try:
            shutil.rmtree('results')
            print(f"The directory results has been removed successfully")
        except Exception as e:
            print(f"Error: {e}")
    app.run(debug=False, host='0.0.0.0', port=8001)


