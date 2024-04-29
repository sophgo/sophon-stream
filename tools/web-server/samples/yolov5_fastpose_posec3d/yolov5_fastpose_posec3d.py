from samples.yolov5_fastpose_posec3d.config_logic import *
import json

def yolov5_fastpose_posec3d_build_config(algorithm_name,stream_path,data,port,i):
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

def yolov5_fastpose_posec3d_trans_json(json_data,task_id,Type,up_list):

    results={}
    frame_id=int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"]=frame_id    
    src_base64=json_data["mFrame"]["mSpData"]
    results["SceneImageBase64"]=src_base64
    results["AnalyzeEvents"]=[]
    results["TaskID"]=str(task_id)

    boxes=[]
    if("mRecognizedObjectMetadatas" in json_data.keys() and "mDetectedObjectMetadatas" in json_data.keys()):
        print(json_data["mRecognizedObjectMetadatas"][0])
        for indx in range(1):
            tmp=json_data["mDetectedObjectMetadatas"][indx]
            tmp2=json_data["mRecognizedObjectMetadatas"][0]
            if tmp2["mLabelName"] in up_list:
                result={}
                x1,y1=tmp["mBox"]["mX"],tmp["mBox"]["mY"]
                x2=x1+tmp["mBox"]["mWidth"]
                y2=y1+tmp["mBox"]["mHeight"]
                boxes.append((x1,y1,x2,y2))
                # result["ImageBase64"]=tmp2["mFrame"]["mSpData"]
                result["Box"]={"LeftTopY": y1,
                                "RightBtmY": y2,
                                "LeftTopX": x1,
                                "RightBtmX": x2 }
                result["Type"]=Type
                result["Extend"]={}
                result["Extend"]["pose"]=tmp2["mLabelName"]
                results["AnalyzeEvents"].append(result)
                
    return results
                
def yolov5_fastpose_posec3d_logic(json_data,up_list,rm_list):
    if("mRecognizedObjectMetadatas" in json_data.keys()):
        names=[str(i["mLabelName"]) for i in json_data["mRecognizedObjectMetadatas"]]
    else:
        names=[]
    for name in names:
        if name in yolov5_fastpose_posec3d_infos.keys():
            yolov5_fastpose_posec3d_infos[name]["in"]+=1
            if yolov5_fastpose_posec3d_infos[name]["in"]==yolov5_fastpose_posec3d_in_thresh:
                up_list.append(name)      
        else :
            yolov5_fastpose_posec3d_infos[name]={}
            yolov5_fastpose_posec3d_infos[name]["in"]=1
            if yolov5_fastpose_posec3d_infos[name]["in"]==yolov5_fastpose_posec3d_in_thresh:
                up_list.append(name)
    for key in yolov5_fastpose_posec3d_infos.keys():
        if key not in names:
            if "out" in yolov5_fastpose_posec3d_infos[key].keys():
                yolov5_fastpose_posec3d_infos[key]["out"]+=1
                if yolov5_fastpose_posec3d_infos[key]["out"]>=yolov5_fastpose_posec3d_out_thresh:
                    rm_list.append(key)          
            else:
                yolov5_fastpose_posec3d_infos[key]["out"]=1
    for key in rm_list:
        del yolov5_fastpose_posec3d_infos[key]