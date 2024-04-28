from flask import Flask, request, jsonify
import json
import io
import os
from io import BytesIO
import base64
import requests
import argparse
from datetime import datetime
from config_algorithm import *
algorithms=Algorithms()


app = Flask(__name__)

idx=0
url = "http://0.0.0.0:8001"
result_url=""
url_len=0
result_list=[]
task_id=0
Type=0
id=0
src_id=""
# 定义请求头（如果有的话）
headers = {
    "Content-Type": "application/json"
}

@app.route('/flask_test/', methods=['POST'])
def build_result():
    global Type,idx
    json_data = request.json
    # print(json_data["error"])
    if not os.path.exists("results/"):
            os.makedirs("results/")
    now = datetime.now()
    error_time = now.strftime("%Y-%m-%d %H:%M:%S")
    # with open(str(error_time)+"a.txt", 'w') as file:
    #     json.dump(json_data, file, indent=2)
    if("error" in json_data.keys()):
        # 获取当前时间
        now = datetime.now()
        error_time = now.strftime("%Y-%m-%d %H:%M:%S")
        with open("results/"+str(task_id)+"_"+str(id)+"_"+str(error_time)+'_error'+".json", 'w') as file:
            json.dump(json_data, file, indent=2)
        print(json_data["error"])
        return jsonify({"message": "error", "response": -1})
    frame_id=int(json_data["mFrame"]["mFrameId"])

    # logic 
    # todo
    up_list=[]
    rm_list=[]
    algorithm_name=map_type[Type]
    algorithm_logic=getattr(algorithms,algorithm_name+'_logic')
    algorithm_logic(json_data,up_list,rm_list)
    # trans json 
    # todo
 
    algorithm_trans_json=getattr(algorithms,algorithm_name+'_trans_json')
    results=algorithm_trans_json(json_data,task_id,Type,up_list)
    print(up_list)
    results["SrcID"]=src_id
    if(len(up_list)):
        response = requests.post(result_list[idx%url_len], json=[results], headers=headers)
        idx+=1
        idx%=url_len
        # print(response)
        # if not os.path.exists("results/"):
        #     os.makedirs("results/")
        # with open("results/"+str(task_id)+'_'+str(id)+'_'+str(frame_id)+".json", 'w') as file:
        #     json.dump([results], file, indent=2)
    # print(json_data)
    return jsonify({"message": "Request received and processed successfully", "response": 1})

def argsparser():
    parser = argparse.ArgumentParser(prog=__file__)
    parser.add_argument('--task_id', type=str, default="0", help='id of task')
    parser.add_argument('--type', type=int, default=0, help='type of algorithm')
    parser.add_argument('--host', type=str, default="0.0.0.0", help='ip of host')
    parser.add_argument('--port', type=int, default=11100, help='port of host')
    parser.add_argument('--id', type=int, default=0, help='port of host')
    parser.add_argument('--url', type=str, default="", help='report url')
    parser.add_argument('--src_id', type=str, default="", help='src_id')

    args = parser.parse_args()
    return args
if __name__=="__main__":
    args = argsparser()
    task_id=args.task_id
    id=args.id
    Type=args.type
    url="http://"+args.host+":"+str(args.port)
    result_url=args.url
    src_id=args.src_id
    result_list=eval(result_url)
    url_len=len(result_list)
    app.run(debug=False, host='0.0.0.0', port=args.port)


