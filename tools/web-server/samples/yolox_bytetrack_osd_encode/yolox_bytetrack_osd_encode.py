from samples.yolox_bytetrack_osd_encode.config_logic import *
import json
import base64
from io import BytesIO
from PIL import Image
def yolox_bytetrack_osd_encode_build_config(algorithm_name,stream_path,data,port,i):
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
    json_data["configure"]["rules"][0]["filters"][0]["areas"]=[]
    if(data["Algorithm"][i]["DetectInfos"]!=None):
        for detectinfoid in range(len(data["Algorithm"][i]["DetectInfos"])):
            area=[{"left":i['X'],"top":i["Y"]} for i in data["Algorithm"][i]["DetectInfos"][detectinfoid]["HotArea"]]
            json_data["configure"]["rules"][0]["filters"][0]["areas"].append(area)
            head=data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineStart"]
            end=data["Algorithm"][i]["DetectInfos"][detectinfoid]["TripWire"]["LineEnd"]
            line=[{"left":head['X'],"top":head["Y"]},{"left":end['X'],"top":end["Y"]}]
            json_data["configure"]["rules"][0]["filters"][0]["areas"].append(line)

    
    else:
        if("roi"in json_data["configure"].keys()):
            del json_data["configure"]["roi"]
        with open(det_config_path, 'w') as file:
            json.dump(json_data, file, indent=2)
    with open(filter_config_path, 'w') as file:
        json.dump(json_data, file, indent=2)
    return demo_config_path

def yolox_bytetrack_osd_encode_trans_json(json_data,task_id,Type,up_list):
    results={}
    frame_id=int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"]=frame_id    
    src_base64=json_data["mFrame"]["mSpData"]
    results["SceneImageBase64"]=src_base64
    results["AnalyzeEvents"]=[]
    results["TaskID"]=str(task_id)
    image_data = base64.b64decode(src_base64)
    # 使用Pillow打开图片
    image = Image.open(BytesIO(image_data))
    width, height = image.size

    boxes=[]
    if("mTrackedObjectMetadatas" in json_data.keys()):
        for indx in range(len(json_data["mDetectedObjectMetadatas"])):
            tmp=json_data["mDetectedObjectMetadatas"][indx]
            tmp2=json_data["mTrackedObjectMetadatas"][indx]
            if str(tmp2["mTrackId"]) in up_list:
                result={}
                x1,y1=tmp["mBox"]["mX"],tmp["mBox"]["mY"]
                x2=x1+tmp["mBox"]["mWidth"]
                y2=y1+tmp["mBox"]["mHeight"]
                boxes.append((x1,y1,x2,y2))
                crop_box = crop_target_square(width, height, x1, y1, x2, y2)
                cropped_image = image.crop(crop_box)

                # 使用不同的 BytesIO 对象
                buffer = BytesIO()
                cropped_image.save(buffer, format="JPEG")  # 选择格式
                # 将buffer内容转换为base64格式
                buffer.seek(0)
                small_image = base64.b64encode(buffer.getvalue()).decode("utf-8")
                result["ImageBase64"]=small_image
                result["Box"]={"LeftTopY": y1,
                                "RightBtmY": y2,
                                "LeftTopX": x1,
                                "RightBtmX": x2 }
                result["Type"]=Type
                # result["Extend"]={}
                # result["Extend"]["VehicleLicense"]=tmp2["mRecognizedObjectMetadatas"][0]["mLabelName"]
                results["AnalyzeEvents"].append(result)
    return results
                
def yolox_bytetrack_osd_encode_logic(json_data,up_list,rm_list):

    if("mTrackedObjectMetadatas" in json_data.keys()):
        for i in json_data["mTrackedObjectMetadatas"]:
            up_list.append(str(i["mTrackId"]))
    else:
        up_list=[]
    # for name in names:
    #     if name in yolox_bytetrack_osd_encode_infos.keys():
    #         yolox_bytetrack_osd_encode_infos[name]["in"]+=1
    #         if yolox_bytetrack_osd_encode_infos[name]["in"]==yolox_bytetrack_osd_encode_in_thresh:
    #             up_list.append(name)      
    #     else :
    #         yolox_bytetrack_osd_encode_infos[name]={}
    #         yolox_bytetrack_osd_encode_infos[name]["in"]=1
    #         if yolox_bytetrack_osd_encode_infos[name]["in"]==yolox_bytetrack_osd_encode_in_thresh:
    #             up_list.append(name)
    # for key in yolox_bytetrack_osd_encode_infos.keys():
    #     if key not in names:
    #         if "out" in yolox_bytetrack_osd_encode_infos[key].keys():
    #             yolox_bytetrack_osd_encode_infos[key]["out"]+=1
    #             if yolox_bytetrack_osd_encode_infos[key]["out"]>=yolox_bytetrack_osd_encode_out_thresh:
    #                 rm_list.append(key)          
    #         else:
    #             yolox_bytetrack_osd_encode_infos[key]["out"]=1
    # for key in rm_list:
    #     del yolox_bytetrack_osd_encode_infos[key]
def crop_target_square(width, height, x1, y1, x2, y2):
    # 计算目标正方形的中心坐标
    center_x = (x1 + x2) // 2
    center_y = (y1 + y2) // 2

    # 计算正方形的一半边长
    half_size = max(center_x - x1, x2 - center_x, center_y - y1, y2 - center_y)

    # 计算正方形的边界
    left = max(0, center_x - half_size - 20)
    top = max(0, center_y - half_size - 20)
    right = min(width, center_x + half_size + 20)
    bottom = min(height, center_y + half_size + 20)

    # 返回新的四个坐标
    return left, top, right, bottom
