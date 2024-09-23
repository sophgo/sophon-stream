import json
import base64
from io import BytesIO
from PIL import Image, ImageDraw


def openpose_build_config(taskId, channleId, algorithm_name, data):
    config_path = "tasks/" + taskId + "/config/"

    demo_config_path = config_path + algorithm_name + "_demo.json"
    engine_config_path = config_path + algorithm_name + "_group.json"
    # print(engine_config_path)
    with open(demo_config_path, "r") as file:
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

    filter_config_path = config_path + "filter.json"
    with open(filter_config_path, "r") as file:
        json_data = json.load(file)

    json_data["configure"]["rules"][0]["channel_id"] = channleId
    json_data["configure"]["rules"][0]["filters"][0]["areas"] = []
    json_data["configure"]["rules"][0]["filters"][0]["type"] = 2
    if "TrackInterval" in data["Algorithm"][0]:
        json_data["configure"]["rules"][0]["filters"][0]["alert_first_frame"] = data[
            "Algorithm"
        ][0]["TrackInterval"]
    if "AlarmInterval" in data["Algorithm"][0]:
        json_data["configure"]["rules"][0]["filters"][0]["alert_frame_skip_nums"] = (
            data["Algorithm"][0]["AlarmInterval"]
        )
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
        with open(engine_config_path, "r") as file:
            json_data = json.load(file)
        if "roi" in json_data["configure"].keys():
            del json_data["configure"]["roi"]
        with open(engine_config_path, "w") as file:
            json.dump(json_data, file, indent=2)

    return demo_config_path


def openpose_trans_json(json_data, task_id, Type):
    results = {}
    frame_id = int(json_data["mFrame"]["mFrameId"])
    results["FrameIndex"] = frame_id
    src_base64 = json_data["mFrame"]["mSpData"]
    results["SceneImageBase64"] = src_base64
    results["AnalyzeEvents"] = []
    results["TaskID"] = str(task_id)

    if (
        len(json_data["mPosedObjectMetadatas"]) > 0
        and len(results["AnalyzeEvents"]) == 0
    ):
        # 解码base64图片
        image_data = base64.b64decode(src_base64)
        # 使用Pillow打开图片
        image = Image.open(BytesIO(image_data))
        # 获取图像分辨率
        width, height = image.size

        for indx, item in enumerate(json_data["mPosedObjectMetadatas"]):

            x1, y1, x2, y2 = draw_keypoints(image, item["keypoints"], width, height)
            cropped_image = image.crop((x1, y1, x2, y2))

            buffer = BytesIO()
            cropped_image.save(buffer, format="JPEG")
            # 将buffer内容转换为base64格式
            buffer.seek(0)
            small_image = base64.b64encode(buffer.getvalue()).decode("utf-8")

            result = {
                "ImageBase64": small_image,
                "Box": {
                    "LeftTopY": int(y1),
                    "RightBtmY": int(y2),
                    "LeftTopX": int(x1),
                    "RightBtmX": int(x2),
                },
                "Type": Type,
                "Extend": {},
            }

            results["AnalyzeEvents"].append(result)
    return results


def draw_keypoints(image, keypoints, width, height):
    draw = ImageDraw.Draw(image)

    min_x, min_y = int("10000"), int("10000")
    max_x, max_y = int("0"), int("0")

    for index in range(0, len(keypoints), 3):
        if keypoints[index] == 0 and keypoints[index + 1] == 0:
            continue
        if keypoints[index + 2] < 0.1:
            continue

        start_x = keypoints[index]
        start_y = keypoints[index + 1]

        min_x = min(min_x, start_x)
        min_y = min(min_y, start_y)
        max_x = max(max_x, start_x)
        max_y = max(max_y, start_y)

        # 绘制小圆圈
        radius = 3  # 圆圈半径
        draw.ellipse(
            (start_x - radius, start_y - radius, start_x + radius, start_y + radius),
            fill="red",
        )

    left = max(0, min_x - 10)
    top = max(0, min_y - 10)
    right = min(width, max_x + 10)
    bottom = min(height, max_y + 10)

    return left, top, right, bottom
