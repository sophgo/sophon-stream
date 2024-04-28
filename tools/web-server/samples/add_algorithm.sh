#!/bin/bash

# 检查是否传入了参数 \$1
if [[ -z $1 && -z $2 ]]; then
    echo "Usage: bash add_algorithm.sh  <algorithm> <type>"
    exit 1
else
    echo "Parameter 1 is set to $1.Parameter 2 is set to $2."
fi

# 你的其余脚本可以放在这里

algorithm=$1
type=$2
if [ -d "$algorithm" ]; then
    echo "Directory $directory already exists."
else
    cp -r template $algorithm
    mv $algorithm/template.py $algorithm/$algorithm.py
fi


# 使用 sed 进行替换
sed -i "s/template/$algorithm/g" "$algorithm/$algorithm.py"
sed -i "s/template/$algorithm/g" "$algorithm/config_logic.py"



sed -i "1 a import samples.${algorithm}.${algorithm} as ${algorithm}" ../config_algorithm.py

# 更新Algorithms类
sed -i "/class Algorithms:/a \ \ \ \ def ${algorithm}_logic(self,json_data,up_list,rm_list):\n\ \ \ \ \ \ \ \ return ${algorithm}.${algorithm}_logic(json_data,up_list,rm_list)" ../config_algorithm.py

sed -i "/class Algorithms:/a \ \ \ \ def ${algorithm}_trans_json(self,json_data,task_id,Type,up_list):\n\ \ \ \ \ \ \ \ return ${algorithm}.${algorithm}_trans_json(json_data,task_id,Type,up_list)" ../config_algorithm.py

sed -i "/class Algorithms:/a \ \ \ \ def ${algorithm}_build_config(self,algorithm_name,stream_path,data,port,i):\n\ \ \ \ \ \ \ \ return ${algorithm}.${algorithm}_build_config(algorithm_name,stream_path,data,port,i)" ../config_algorithm.py


# 更新map_type字典
new_map_entry="$type:'${algorithm}'"
sed -i "/map_type={/s/}/, $new_map_entry}/" ../config_algorithm.py

echo "New algorithm '${algorithm}' added to Algorithms.py and file structure created."


# 输出结果
echo "Add $algorithm success!"