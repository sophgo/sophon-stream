from samples.retinaface.config_logic import *
import json

def retinaface_build_config(algorithm_name,stream_path,data,port):
    config_path=stream_path+'/samples/'+algorithm_name+'/config/'
   
    demo_config_path=config_path+algorithm_name+'_demo.json'
    http_config_path=config_path+'http_push.json'
    # det_config_path=config_path+'yolov5_group.json'
    with open(demo_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["channels"]=[json_data["channels"][0]]
    json_data["channels"][0]["url"]=data["InputSrc"]["StreamSrc"]["Address"]
    json_data["channels"][0]["sample_interval"]=data["Algorithm"][0]["DetectInterval"]
    json_data["channels"][0]["source_type"]=data["InputSrc"]["StreamSrc"]["Address"][:4].upper()
    with open(demo_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
    with open(http_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["configure"]["path"]="/flask_test/"+data['TaskID']
    json_data["configure"]["port"]=port

    with open(http_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
   
    return demo_config_path

def retinaface_trans_json(json_data,task_id,Type,up_list):
    results={}

                
def retinaface_logic(json_data,up_list,rm_list):
    for key in json_data.keys():
        print(key)