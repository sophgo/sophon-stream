from samples.license_area_intrusion.config_logic import *
import json
import os
import glob
import copy
def license_area_intrusion_build_config(algorithm_name,stream_path,data,port,i,is_single_process):
    config_path=stream_path+'/samples/'+algorithm_name+'/config/'
 
    demo_config_path=config_path+algorithm_name+'_demo.json'
    http_config_path=config_path+'http_push.json'
    det_config_path=glob.glob(os.path.join(config_path, 'yolo*_group.json'))[0]
    graph_path=config_path+'engine_group.json'
    filter_config_path=config_path+'filter.json'
    with open(demo_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    if not is_single_process: 
        json_data["channels"]=[]
    json_data["channels"].append({})
    channel_num=len(json_data["channels"])
    i=channel_num-1
    json_data["channels"][i]["channel_id"]=i

    json_data["channels"][i]["url"]=data["InputSrc"]["StreamSrc"]["Address"]
    
    json_data["channels"][i]["sample_interval"]=data["Algorithm"][0]["DetectInterval"]
    if(data["InputSrc"]["StreamSrc"]["Address"][:1]=="/"):
        json_data["channels"][i]["source_type"]="VIDEO"
    elif(data["InputSrc"]["StreamSrc"]["Address"][:7]=="gb28181"):
        json_data["channels"][i]["source_type"]=data["InputSrc"]["StreamSrc"]["Address"][:7].upper()
    else:
        json_data["channels"][i]["source_type"]=data["InputSrc"]["StreamSrc"]["Address"][:4].upper()

    json_data["channels"][i]["fps"]=25
    json_data["channels"][i]["loop_num"]=-1
    i=0
   

    json_data["http_report"]={}
    json_data["http_report"]["ip"]="0.0.0.0"
    json_data["http_report"]["port"]=port
    json_data["http_report"]["path"]="/flask_test/"

    with open(demo_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
    with open(http_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    # print(data["InputSrc"]["StreamSrc"]["Address"])
    json_data["configure"]["path"]="/flask_test/"
    json_data["configure"]["port"]=port
    json_data["thread_number"]=channel_num
    with open(http_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
        
        

    
    with open(det_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["configure"]["threshold_conf"]=1.0*data["Algorithm"][i]["threshold"]/100.0
    json_data["thread_number"]=channel_num
    with open(det_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
        
    
    converger_config_path=config_path+'converger.json' 
    with open(converger_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["thread_number"]=channel_num
    with open(converger_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
    
    distributor_time_class_config_path=config_path+'distributor_time_class.json'     
    with open(distributor_time_class_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["thread_number"]=channel_num
    with open(distributor_time_class_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)    
        
    lprnet_group_config_path=config_path+'lprnet_group.json'     

    with open(lprnet_group_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["thread_number"]=channel_num
    with open(lprnet_group_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
        
    with open(filter_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    json_data["thread_number"]=channel_num
    filter_=json_data["configure"]["rules"][0]
    
    filter_["filters"][0]["areas"]=[]
    filter_["filters"][0]["type"]=0
    filter_["filters"][0]["alert_first_frame"]=data["Algorithm"][i]["TrackInterval"]
    filter_["filters"][0]["alert_frame_skip_nums"]=data["Algorithm"][i]["AlarmInterval"]

    if(data["Algorithm"][i]["DetectInfos"]!=None):
        for detectinfoid in range(len(data["Algorithm"][i]["DetectInfos"])):
            area=[{"left":i['X'],"top":i["Y"]} for i in data["Algorithm"][i]["DetectInfos"][detectinfoid]["HotArea"]]
            filter_["filters"][0]["areas"].append(area)
            # head=data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineStart"]
            # end=data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineEnd"]
            # line=[{"left":head['X'],"top":head["Y"]},{"left":end['X'],"top":end["Y"]}]
            # json_data["configure"]["rules"][0]["filters"][0]["areas"].append(line)
            
    
    else:
        with open(det_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
            json_data = json.load(file)
        if("roi"in json_data["configure"].keys()):
            del json_data["configure"]["roi"]
        with open(det_config_path, 'w') as file:
            json.dump(json_data, file, indent=2)
    json_data["configure"]["rules"]=[]
    for channel_idx in range(channel_num):
        tmp_filter=copy.deepcopy(filter_)
        # tmp_filter["channel_id"]=channel_idx
        tmp_filter["channel_id"]=channel_idx
        json_data["configure"]["rules"].append(tmp_filter)
    with open(filter_config_path, 'w') as file:
                    json.dump(json_data, file, indent=2)
    return demo_config_path

def license_area_intrusion_trans_json(json_data,task_id,Type,up_list):
    results={}
    frame_id=int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"]=frame_id
        # print(json_data.keys())
    
    src_base64=json_data["mFrame"]["mSpData"]
    results["SceneImageBase64"]=src_base64
    results["AnalyzeEvents"]=[]
    results["TaskID"]=str(task_id)
    # save_base64_image(src_base64,'gg3.jpg')
    base64s=[]

    boxes=[]
    if("mSubObjectMetadatas" in json_data.keys()):
        for indx in range(len(json_data["mDetectedObjectMetadatas"])):
            tmp=json_data["mDetectedObjectMetadatas"][indx]
            tmp2=json_data["mSubObjectMetadatas"][indx]
            if tmp2["mRecognizedObjectMetadatas"][0]["mLabelName"] in up_list:
                result={}
                x1,y1=tmp["mBox"]["mX"],tmp["mBox"]["mY"]
                x2=x1+tmp["mBox"]["mWidth"]
                y2=y1+tmp["mBox"]["mHeight"]
                boxes.append((x1,y1,x2,y2))
                    # print(tmp2["mFrame"]["mSpData"])
                    # save_base64_image(tmp2["mFrame"]["mSpData"],'gg.jpg')
                    # result["ImageBase64"]=crop_image_base64(src_base64, (x1,y1,x2,y2))
                result["ImageBase64"]=tmp2["mFrame"]["mSpData"]
                # print(result["ImageBase64"])
                # save_base64_image(result["ImageBase64"],'gg2.jpg')

                result["Box"]={"LeftTopY": y1,
                                "RightBtmY": y2,
                                "LeftTopX": x1,
                                "RightBtmX": x2 }
                result["Type"]=Type
                result["Extend"]={}
                result["Extend"]["VehicleLicense"]=tmp2["mRecognizedObjectMetadatas"][0]["mLabelName"]
                results["AnalyzeEvents"].append(result)
    return results
                
def license_area_intrusion_logic(json_data,up_list,rm_list):
    if("mSubObjectMetadatas" in json_data.keys()):
        names=[str(i["mRecognizedObjectMetadatas"][0]["mLabelName"]) for i in json_data["mSubObjectMetadatas"]]
    else:
        names=[]
    for name in names:
        up_list.append(name)