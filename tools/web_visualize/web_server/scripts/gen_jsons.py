#//===----------------------------------------------------------------------===//
#//
#// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
#//
#// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
#// third-party components.
#//
#//===----------------------------------------------------------------------===//

import os
import json

# 定义简化后的目录列表
directories = [
    "yolov5",
    "yolox",
    "resnet",
    "bytetrack",
    "yolov5_bytetrack_distributor_resnet_converger",
    "yolox_bytetrack_osd_encode"
]

base_path = "../../../../samples/"
config_path = "config"

all_files = []
json_id = 0
# 遍历各个目录并提取文件名
for pipeline_id, directory in enumerate(directories):
    full_directory = os.path.join(base_path, directory, config_path)
    json_list = []

    for _, file_name in enumerate(sorted(os.listdir(full_directory))):
        if file_name.endswith(".json"):
            json_list.append({
                "json_id": str(json_id),
                "json_name": file_name
            })
        json_id += 1

    # 构建包含信息的JSON对象
    pipeline_info = {
        "pipeline_id": pipeline_id,
        "pipeline_name": directory,
        "is_running": False,
        "json_list": json_list
    }

    all_files.append(pipeline_info)

with open("../config/all_files_preview.json", "w") as outfile:
    json.dump(all_files, outfile, indent=4)
