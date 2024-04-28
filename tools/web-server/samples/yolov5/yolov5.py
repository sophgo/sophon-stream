from samples.yolov5.config_logic import *
import base64
from io import BytesIO
from PIL import Image
import json

yolov5_dict = {
    0: 'person',
    1: 'bicycle',
    2: 'car',
    3: 'motorbike',
    4: 'aeroplane',
    5: 'bus',
    6: 'train',
    7: 'truck',
    8: 'boat',
    9: 'traffic light',
    10: 'fire hydrant',
    11: 'stop sign',
    12: 'parking meter',
    13: 'bench',
    14: 'bird',
    15: 'cat',
    16: 'dog',
    17: 'horse',
    18: 'sheep',
    19: 'cow',
    20: 'elephant',
    21: 'bear',
    22: 'zebra',
    23: 'giraffe',
    24: 'backpack',
    25: 'umbrella',
    26: 'handbag',
    27: 'tie',
    28: 'suitcase',
    29: 'frisbee',
    30: 'skis',
    31: 'snowboard',
    32: 'sports ball',
    33: 'kite',
    34: 'baseball bat',
    35: 'baseball glove',
    36: 'skateboard',
    37: 'surfboard',
    38: 'tennis racket',
    39: 'bottle',
    40: 'wine glass',
    41: 'cup',
    42: 'fork',
    43: 'knife',
    44: 'spoon',
    45: 'bowl',
    46: 'banana',
    47: 'apple',
    48: 'sandwich',
    49: 'orange',
    50: 'broccoli',
    51: 'carrot',
    52: 'hot dog',
    53: 'pizza',
    54: 'donut',
    55: 'cake',
    56: 'chair',
    57: 'sofa',
    58: 'pottedplant',
    59: 'bed',
    60: 'diningtable',
    61: 'toilet',
    62: 'tvmonitor',
    63: 'laptop',
    64: 'mouse',
    65: 'remote',
    66: 'keyboard',
    67: 'cell phone',
    68: 'microwave',
    69: 'oven',
    70: 'toaster',
    71: 'sink',
    72: 'refrigerator',
    73: 'book',
    74: 'clock',
    75: 'vase',
    76: 'scissors',
    77: 'teddy bear',
    78: 'hair drier',
    79: 'toothbrush',
}

def yolov5_build_config(algorithm_name,stream_path,data,port,i):
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

def yolov5_trans_json(json_data,task_id,Type,up_list):
    results={}
    frame_id=int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"]=frame_id    
    src_base64=json_data["mFrame"]["mSpData"]
    # src_base64= " "
    results["SceneImageBase64"]=src_base64
    results["AnalyzeEvents"]=[]
    results["TaskID"]=str(task_id)

    # 解码base64图片
    image_data = base64.b64decode(src_base64)
    # 使用Pillow打开图片
    image = Image.open(BytesIO(image_data))
    # 获取图像分辨率
    width, height = image.size

    boxes=[]
    
    result={}
    image_pre = ""
    if len(json_data["mDetectedObjectMetadatas"]) > 0:
        for indx, item in enumerate(json_data["mDetectedObjectMetadatas"]):
            x1, y1 = item["mBox"]["mX"], item["mBox"]["mY"]
            x2 = x1 + item["mBox"]["mWidth"]
            y2 = y1 + item["mBox"]["mHeight"]
            boxes.append((x1, y1, x2, y2))

            # 根据坐标裁剪图片
            
            crop_box = crop_target_square(width, height, x1, y1, x2, y2)
            cropped_image = image.crop(crop_box)

            # 使用不同的 BytesIO 对象
            buffer = BytesIO()
            cropped_image.save(buffer, format="JPEG")  # 选择格式
            # 将buffer内容转换为base64格式
            buffer.seek(0)
            small_image = base64.b64encode(buffer.getvalue()).decode("utf-8")

            result = {
                "ImageBase64": small_image,
                "Box": {
                    "LeftTopY": y1,
                    "RightBtmY": y2,
                    "LeftTopX": x1,
                    "RightBtmX": x2
                },
                "Type": Type,
                "Extend": {
                    "类别": yolov5_dict.get(item["mClassify"], 'Not found'),
                    "置信度":str(round(item["mScores"][0]*100,2)) + "%"
                }
            }

            results["AnalyzeEvents"].append(result)

    return results
                
def yolov5_logic(json_data,up_list,rm_list):
    if("mDetectedObjectMetadatas" in json_data.keys()):
        tmp = json_data["mDetectedObjectMetadatas"]
        # mClassify_list = [item["mClassify"] for tmp in tmp for item in tmp]
    else:
        names=[]
    up_list.append(1)    
    # print(json_data["mDetectedObjectMetadatas"])
    # for key in json_data.keys():
    #     print(key)

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
