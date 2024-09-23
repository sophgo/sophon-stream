import os
import subprocess
import json

def build_task_config(src_folder, dst_folder, new_stream_path):
    if not os.path.exists(dst_folder):
        os.makedirs(dst_folder)

    subprocess.run(['cp', '-r', src_folder, dst_folder], check=True)
    config_dst_folder = os.path.join(dst_folder, 'config')
    
    for file_name in os.listdir(config_dst_folder):
        if file_name.endswith('.json'):
            file_path = os.path.join(config_dst_folder, file_name)
            
            with open(file_path, 'r', encoding='utf-8') as f:
                file_content = f.read()
            
            # 替换 {Stream_path} 字符串
            updated_content = file_content.replace('{Stream_path}', new_stream_path)
            
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(updated_content)

def check_json_files(directory):
    """
    检查所有 JSON 文件，查找对应文件是否存在。
    """
    def check_file_exists(filepath):
        """检查文件路径是否存在"""
        return os.path.isfile(filepath)

    def find_fields(data, fields, found_fields):
        """递归查找 JSON 对象中的特定字段"""
        if isinstance(data, dict):
            for key, value in data.items():
                if key in fields:
                    found_fields[key] = value
                if isinstance(value, (dict, list)):
                    find_fields(value, fields, found_fields)
        elif isinstance(data, list):
            for item in data:
                find_fields(item, fields, found_fields)

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.json'):
                file_path = os.path.join(root, file)
                with open(file_path, 'r', encoding='utf-8') as f:
                    try:
                        data = json.load(f)
                        found_fields = {}
                        fields = ["model_path", "shared_object", "class_names"]
                        find_fields(data, fields, found_fields)
                        for field, value in found_fields.items():
                            if isinstance(value, str) and check_file_exists(value):
                                continue
                            else:
                                return f'error: 文件 {file_path} 中 {value} 不存在'
                    except json.JSONDecodeError:
                        print(f'Error decoding JSON in file: {file_path}')
    return ""


if __name__=="__main__":
    src_folder = 'samples/yolov5/config'
    dst_folder = 'tasks/1002'
    new_stream_path = '/data/wyf/sophon-stream'

    # build_task_config(src_folder, dst_folder, new_stream_path)

    directory_to_check = 'tasks/1002/config'
    res = check_json_files(directory_to_check)
    print(res)
