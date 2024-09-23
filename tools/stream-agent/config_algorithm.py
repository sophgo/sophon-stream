import samples.license_area_intrusion.license_area_intrusion as license_area_intrusion
import samples.yolov5.yolov5 as yolov5
import samples.yolov8.yolov8 as yolov8
import samples.tripwire.tripwire as tripwire
import samples.openpose.openpose as openpose

map_type = {1: "tripwire", 2: "license_area_intrusion", 3: "yolov5", 4: "yolov8", 5: "openpose"}


class Algorithms:
    def yolov5_build_config(self, taskId, channleId, algorithm_name, data):
        return yolov5.yolov5_build_config(taskId, channleId, algorithm_name, data)

    def yolov5_trans_json(self, json_data, task_id, Type):
        return yolov5.yolov5_trans_json(json_data, task_id, Type)
    
    def yolov8_build_config(self, taskId, channleId, algorithm_name, data):
        return yolov8.yolov8_build_config(taskId, channleId, algorithm_name, data)

    def yolov8_trans_json(self, json_data, task_id, Type):
        return yolov8.yolov8_trans_json(json_data, task_id, Type)
    

    def tripwire_build_config(self, taskId, channleId, algorithm_name, data):
        return tripwire.tripwire_build_config(taskId, channleId, algorithm_name, data)

    def tripwire_trans_json(self, json_data, task_id, Type):
        return tripwire.tripwire_trans_json(json_data, task_id, Type)

    def license_area_intrusion_build_config(self, taskId, channleId, algorithm_name, data):
        return license_area_intrusion.license_area_intrusion_build_config(
            taskId, channleId, algorithm_name, data
        )

    def license_area_intrusion_trans_json(self, json_data, task_id, Type):
        return license_area_intrusion.license_area_intrusion_trans_json(
            json_data, task_id, Type
        )
    
    def openpose_build_config(self, taskId, channleId, algorithm_name, data):
        return openpose.openpose_build_config(taskId, channleId, algorithm_name, data)

    def openpose_trans_json(self, json_data, task_id, Type):
        return openpose.openpose_trans_json(json_data, task_id, Type)