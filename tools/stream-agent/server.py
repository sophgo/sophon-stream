from flask import Flask, request, jsonify
import argparse
import subprocess
import os
from config_algorithm import *
import common
import time
import requests
import threading
import json

app = Flask(__name__)
stream_path = ""  # stream绝对路径
current_directory = os.getcwd()
process_pools = {}  # stream运行线程，[taskId](process)
taskId_map = {}  # [taskId](channelId, srcId, algorithm_name, algoType) 在接收函数中，根据channelId获取对应的taskId等信息
report_map = {}  # [taskId](report_urls)

algorithms = Algorithms()  # 调用的samples中函数
headers = {"Content-Type": "application/json"}


def build_start(data):
    """
    在tasks文件夹中创建任务文件 拷贝config配置
    修改json文件中对应配置
    启动stream进程
    """

    taskId = data["TaskID"]
    srcId = data["InputSrc"]["SrcID"]
    config_path = "tasks/" + taskId
    algorithm_name = ""

    res = task_status(taskId)
    if taskId in res and res(taskId)["Status"] == 1:
        return 1

    for i in range(len(data["Algorithm"])):
        algorithm_name = map_type[data["Algorithm"][i]["Type"]]  # yolov5
        if taskId not in taskId_map:
            subprocess.run(["rm", "-rf", config_path], check=True)
            common.build_task_config(
                "samples/" + algorithm_name + "/config", config_path, stream_path
            )
            for channelId in range(100):
                if channelId not in [v[0] for v in taskId_map.values()]:
                    taskId_map[taskId] = (
                        channelId,
                        srcId,
                        algorithm_name,
                        data["Algorithm"][i]["Type"],
                    )
                    break

        elif taskId_map[taskId][2] != algorithm_name:
            subprocess.run(["rm", "-rf", config_path], check=True)

            task_info = list(taskId_map[taskId])
            task_info[2] = algorithm_name
            task_info[3] = data["Algorithm"][i]["Type"]
            # 将列表转换回元组并更新字典
            taskId_map[taskId] = tuple(task_info)

            common.build_task_config(
                "samples/" + algorithm_name + "/config", config_path, stream_path
            )

        with open(config_path + "/" + taskId + "_create.json", "w") as file:
            json.dump(data, file, indent=2)
            

        algorithm_build_config = getattr(algorithms, algorithm_name + "_build_config")
        algorithm_build_config(taskId, taskId_map[taskId][0], algorithm_name, data)
        break  # 当前只支持配置一个算法
    
    check_res = common.check_json_files(config_path)
    if check_res != "":
        print(check_res)
        return 2
    report_map[taskId] = data["Reporting"]["ReportUrlList"]
    os.chdir(config_path)
    print("cd ", config_path, "\n", "Worker process started")
    cmd = [
        stream_path + "/samples/build/main",
        "--demo_config_path=" + "config/" + algorithm_name + "_demo.json",
    ]
    print(cmd)
    log_path = str(taskId) + "_stream.log"
    with open(log_path, "w") as log_file:
        stream_process = subprocess.Popen(
            cmd, shell=False, stdout=log_file, stderr=subprocess.STDOUT
        )
    os.chdir(current_directory)

    process_pools[taskId] = stream_process

    return 0


def alarm_process(channelId, data):
    # 查找 task_id
    taskId = ""
    for task_id, (ch_id, src_id, algo_name, algo_type) in taskId_map.items():
        if ch_id == channelId:
            taskId = task_id
            break

    algorithm_trans_json = getattr(algorithms, taskId_map[taskId][2] + "_trans_json")
    results = algorithm_trans_json(data, taskId, taskId_map[taskId][3])
    results["SrcID"] = src_id

    for item in report_map[taskId]:
        requests.post(item, json=[results], headers=headers)

    return


def task_status(task_id):
    """
    获取指定任务的状态。

    参数:
    task_id (str): 任务的唯一标识符。

    返回:
    dict: 包含任务ID和状态的字典。如果任务存在，返回的字典格式为:
          {"TaskID": task_id, "Status": 1} 表示任务正在运行，
          {"TaskID": task_id, "Status": 0} 表示任务已结束。
          如果任务不存在，返回 {"Status": "No task"}。
    """
    res = {}
    if task_id in process_pools.keys():
        res["TaskID"] = task_id

        status = status = 1 if process_pools[task_id].poll() is None else 0
        res["Status"] = status
        return res
    return res


def get_task_list():
    """
    获取指定任务列表包括状态。
    """
    for task_id in taskId_map.keys():
        print(task_status(task_id))
    print("taskId_map is :", taskId_map)
    ans = []
    for task_id in taskId_map.keys():
        ans.append(task_status(task_id))
    return ans


def del_task(task_id):
    if task_status(task_id)["Status"] == 1:
        process_pools[task_id].kill()
        time.sleep(0.1)
        del taskId_map[task_id]
        del report_map[task_id]
        return 0
    else:
        return 1


@app.route("/task/delete", methods=["POST"])
def task_delete():
    ans = del_task(str(request.json["TaskID"]))
    if ans == 0:
        return jsonify({"Code": 0, "Msg": "success"})
    else:
        return jsonify({"Code": -1, "Msg": "failed"})


@app.route("/task/query", methods=["POST"])
def task_query():
    ans = task_status(str(request.json["TaskID"]))
    if ans["Status"] == 1:
        return jsonify({"Code": 0, "Msg": "success", "Result": ans})
    else:
        return jsonify({"Code": 1, "Msg": "fail"})


@app.route("/task/list", methods=["POST"])
def task_list():
    return jsonify({"Code": 0, "Msg": "success", "Result": get_task_list()})


@app.route("/task/create", methods=["POST"])
def task_create():
    res = build_start(request.json)

    if res == 1:
        return jsonify({"Code": -1, "Msg": "task is running"})
    elif res == 2:
        return jsonify({"Code": -1, "Msg": "file not exit"})

    return jsonify({"Code": 0, "Msg": "success"})


@app.route("/alarm/rev", methods=["POST"])
def alarm_rev():
    json_data = request.json
    channelId = 0
    if "mFrame" in json_data and "mChannelId" in json_data["mFrame"]:
        channelId = json_data["mFrame"]["mChannelId"]
    else:
        return jsonify({"message": "Necessary fields are missing", "response": 0}), 400

    thread = threading.Thread(target=alarm_process, args=(channelId, json_data))
    thread.start()
    return jsonify(
        {"message": "Request received and processed successfully", "response": 1}
    )


def argsparser():
    parser = argparse.ArgumentParser(prog=__file__)
    parser.add_argument(
        "--stream_path",
        type=str,
        default="",
        help="path of stream",
    )

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = argsparser()
    stream_path = args.stream_path

    app.run(debug=False, host="0.0.0.0", port=8001, threaded=True)
