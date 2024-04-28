from samples.license_plate_recognition.config_logic import *
import json

def license_plate_recognition_build_config(algorithm_name,stream_path,data,port,i):
    config_path=stream_path+'/samples/'+algorithm_name+'/config/'
    # stream_run_path=stream_path+"/samples/build"

    # cmd="cp -rf "+config_path+' '+stream_run_path
    # result = subprocess.run(cmd, shell=True)
    # print("Return Code:", result.returncode)
    demo_config_path=config_path+algorithm_name+'_demo.json'
    http_config_path=config_path+'http_push.json'
    det_config_path=config_path+'yolov5_group.json'
    graph_path=config_path+'engine_group.json'
    filter_config_path=config_path+'filter.json'
    with open(demo_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    # print(data["InputSrc"]["StreamSrc"]["Address"])
    json_data["channels"]=[json_data["channels"][0]]
    # json_data["channels"][0]["url"]=data["InputSrc"]["StreamSrc"]["Address"]
    
    json_data["channels"][0]["sample_interval"]=data["Algorithm"][i]["DetectInterval"]
    # if(data["InputSrc"]["StreamSrc"]["Address"][:7]=="gb28181"):
    #     json_data["channels"][0]["source_type"]=data["InputSrc"]["StreamSrc"]["Address"][:7].upper()
    # else:
    #     json_data["channels"][0]["source_type"]=data["InputSrc"]["StreamSrc"]["Address"][:4].upper()

    json_data["channels"][0]["fps"]=25
    json_data["channels"][0]["loop_num"]=2100000

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

    with open(http_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
        
        
    # with open(det_config_path, 'r') as file:
    # # 使用 json.load 将文件内容转换为字典
    #     json_data = json.load(file)
    # if(data["Algorithm"][0]["DetectInfos"]!=None):
    #     sx=min([i['X'] for i in data["Algorithm"][0]["DetectInfos"][0]["HotArea"]])
    #     sy=min([i['Y'] for i in data["Algorithm"][0]["DetectInfos"][0]["HotArea"]])
    #     tx=max([i['X'] for i in data["Algorithm"][0]["DetectInfos"][0]["HotArea"]])
    #     ty=max([i['Y'] for i in data["Algorithm"][0]["DetectInfos"][0]["HotArea"]])
    #     json_data["configure"]["roi"]={"left":sx,"top":sy,"width":tx-sx,"height":ty-sy}
    #     with open(det_config_path, 'w') as file:
    #         json.dump(json_data, file, indent=2)
    # else:
    #     if("roi"in json_data["configure"].keys()):
    #         del json_data["configure"]["roi"]
    #     with open(det_config_path, 'w') as file:
    #         json.dump(json_data, file, indent=2)
    
    with open(filter_config_path, 'r') as file:
    # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)
    if(data["Algorithm"][i]["DetectInfos"]!=None):
        for detectinfoid in range(len(data["Algorithm"][i]["DetectInfos"])):
            area=[{"left":i['X'],"top":i["Y"]} for i in data["Algorithm"][i]["DetectInfos"][detectinfoid]["HotArea"]]
            json_data["configure"]["rules"][0]["filters"][0]["areas"].append(area)
            line=[data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineStart"],data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineEnd"]]
            json_data["configure"]["rules"][0]["filters"][0]["areas"].append(line)

            with open(filter_config_path, 'w') as file:
                json.dump(json_data, file, indent=2)
    else:
        if("roi"in json_data["configure"].keys()):
            del json_data["configure"]["roi"]
        with open(det_config_path, 'w') as file:
            json.dump(json_data, file, indent=2)
    return demo_config_path

def license_plate_recognition_trans_json(json_data,task_id,Type,up_list):
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
                
def license_plate_recognition_logic(json_data,up_list,rm_list):
    if("mSubObjectMetadatas" in json_data.keys()):
        # print(json_data["mSubObjectMetadatas"][0]["mRecognizedObjectMetadatas"][0]["mLabelName"])
        names=[str(i["mRecognizedObjectMetadatas"][0]["mLabelName"]) for i in json_data["mSubObjectMetadatas"]]
    else:
        names=[]
    for name in names:
            # print(name)
            # print(license_plate_recognition_infos.keys())
        # print(license_plate_recognition_infos)
        if name in license_plate_recognition_infos.keys():
            license_plate_recognition_infos[name]["in"]+=1
            # print(license_plate_recognition_infos[name]["in"])
            if license_plate_recognition_infos[name]["in"]==license_plate_recognition_in_thresh:
                up_list.append(name)      
        else :
            license_plate_recognition_infos[name]={}
            license_plate_recognition_infos[name]["in"]=1
            if license_plate_recognition_infos[name]["in"]==license_plate_recognition_in_thresh:
                up_list.append(name)
    # print(license_plate_recognition_infos)
    for key in license_plate_recognition_infos.keys():
        if key not in names:
            if "out" in license_plate_recognition_infos[key].keys():
                license_plate_recognition_infos[key]["out"]+=1
                if license_plate_recognition_infos[key]["out"]>=license_plate_recognition_out_thresh:
                    rm_list.append(key)          
            else:
                license_plate_recognition_infos[key]["out"]=1
    for key in rm_list:
        del license_plate_recognition_infos[key]