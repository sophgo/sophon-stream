import json
import os
import glob


def license_area_intrusion_build_config(taskId, channleId, algorithm_name, data):
    config_path = "tasks/" + taskId + "/config/"

    demo_config_path = config_path + algorithm_name + "_demo.json"
    det_config_path = glob.glob(os.path.join(config_path, "yolo*_group.json"))[0]

    with open(demo_config_path, "r") as file:
        # 使用 json.load 将文件内容转换为字典
        json_data = json.load(file)

    json_data["channels"] = [json_data["channels"][0]]
    json_data["channels"][0]["url"] = data["InputSrc"]["StreamSrc"]["Address"]

    json_data["channels"][0]["sample_interval"] = data["Algorithm"][0]["DetectInterval"]
    json_data["channels"][0]["channel_id"] = channleId

    if data["InputSrc"]["StreamSrc"]["Address"][:1] == "/":
        json_data["channels"][0]["source_type"] = "VIDEO"
    elif data["InputSrc"]["StreamSrc"]["Address"][:7] == "gb28181":
        json_data["channels"][0]["source_type"] = data["InputSrc"]["StreamSrc"][
            "Address"
        ][:7].upper()
    else:
        json_data["channels"][0]["source_type"] = data["InputSrc"]["StreamSrc"][
            "Address"
        ][:4].upper()

    json_data["channels"][0]["fps"] = 25
    json_data["channels"][0]["loop_num"] = 500

    with open(demo_config_path, "w") as file:
        json.dump(json_data, file, indent=2)

    with open(det_config_path, "r") as file:
        json_data = json.load(file)
    if "threshold" in json_data["configure"]:
        json_data["configure"]["threshold_conf"] = (
            1.0 * data["Algorithm"][0]["threshold"] / 100.0
        )
    with open(det_config_path, "w") as file:
        json.dump(json_data, file, indent=2)

    filter_config_path = config_path + "filter.json"
    with open(filter_config_path, "r") as file:
        json_data = json.load(file)

    json_data["configure"]["rules"][0]["channel_id"] = channleId
    json_data["configure"]["rules"][0]["filters"][0]["areas"] = []
    json_data["configure"]["rules"][0]["filters"][0]["type"] = 2
    json_data["configure"]["rules"][0]["filters"][0]["alert_first_frame"] = data[
        "Algorithm"
    ][0]["TrackInterval"]
    
    if "AlarmInterval" in data["Algorithm"][0]:
        json_data["configure"]["rules"][0]["filters"][0]["alert_frame_skip_nums"] = data[
            "Algorithm"
        ][0]["AlarmInterval"]
    if "DetectInfos" in data["Algorithm"][0]:
        for detectinfoid in range(len(data["Algorithm"][0]["DetectInfos"])):
            area = [
                {"left": i["X"], "top": i["Y"]}
                for i in data["Algorithm"][0]["DetectInfos"][detectinfoid]["HotArea"]
            ]
            json_data["configure"]["rules"][0]["filters"][0]["areas"].append(area)

            with open(filter_config_path, "w") as file:
                json.dump(json_data, file, indent=2)

    else:
        with open(det_config_path, "r") as file:
            json_data = json.load(file)
        if "roi" in json_data["configure"].keys():
            del json_data["configure"]["roi"]
        with open(det_config_path, "w") as file:
            json.dump(json_data, file, indent=2)

    return demo_config_path


def license_area_intrusion_trans_json(json_data, task_id, Type):
    results = {}
    frame_id = int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"] = frame_id

    src_base64 = json_data["mFrame"]["mSpData"]
    results["SceneImageBase64"] = src_base64
    results["AnalyzeEvents"] = []
    results["TaskID"] = str(task_id)

    boxes = []
    if "mSubObjectMetadatas" in json_data.keys():
        for indx in range(len(json_data["mDetectedObjectMetadatas"])):
            tmp = json_data["mDetectedObjectMetadatas"][indx]
            tmp2 = json_data["mSubObjectMetadatas"][indx]

            result = {}
            x1, y1 = tmp["mBox"]["mX"], tmp["mBox"]["mY"]
            x2 = x1 + tmp["mBox"]["mWidth"]
            y2 = y1 + tmp["mBox"]["mHeight"]
            boxes.append((x1, y1, x2, y2))
            result["ImageBase64"] = tmp2["mFrame"]["mSpData"]

            result["Box"] = {
                "LeftTopY": y1,
                "RightBtmY": y2,
                "LeftTopX": x1,
                "RightBtmX": x2,
            }
            result["Type"] = Type
            result["Extend"] = {}
            result["Extend"]["VehicleLicense"] = tmp2["mRecognizedObjectMetadatas"][0][
                "mLabelName"
            ]
            results["AnalyzeEvents"].append(result)
    return results
