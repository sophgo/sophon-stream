# web_server

## 1 简介

web_server后端开发程序，主要包含五个核心功能模块：获取pipeline List的GET方法、针对指定pipeline id的GET方法、针对指定pipeline id的PATCH方法、针对指定json id的GET方法、以及针对指定json id的PUT方法，这五个模块组成了web_server后端开发程序的核心功能。通过这些接口，前端或其他客户端可以与后端进行交互，实现对pipeline和json数据的管理与算能demo程序的启停操作。

web_server后端开发程序的这五个模块主要内容包括：

1. 获取pipeline List的GET方法：
   - HTTP方法：GET
   - URL格式： `http://{ip address}:{port}/pipelines`
   - 描述：该模块用于获取所有pipeline的摘要信息。
   - 功能：客户端可以使用此请求获取所有可用的pipeline的摘要信息列表，包括所有pipeline的ID、名称、状态以及对应的json列表等完整内容。

2. 针对指定pipeline id的GET方法：
   - HTTP方法：GET
   - URL格式： `http://{ip address}:{port}/pipelines/{pipeline_id}`
   - 描述：该模块用于从服务器获取指定pipeline ID的详细信息。
   - 功能：客户端可以使用此请求获取特定pipeline ID的所有属性和当前状态，主要有以下四个属性：pipeline_id，pipeline_name，is_running，json_list，该方法得到的数据内容是 [pipeline List GET] 得到的数据的一个实例。

3. 针对指定pipeline id的PATCH方法：
   - HTTP方法：PATCH
   - URL格式： `http://{ip address}:{port}/pipelines/{pipeline_id}`
   - 描述：该模块用于对pipeline进行部分更新，允许客户端向服务器发送部分更改的请求。
   - 功能：支持对pipeline的指定字段进行更新操作，如更新pipeline的状态 is_runnning。本项目目前仅用于修改pipeline的is_running状态，而不需要重新发送整个pipeline的数据。
   
4. 针对指定json id的GET方法：
   - HTTP方法：GET
   - URL格式： `http://{ip address}:{port}/jsons/{json_id}`
   - 描述：该模块用于获取json数据的详细信息。
   - 功能：允许客户端请求服务器返回一个特定json_id文件内json数据的详细信息，可以用于获取json数据的元数据或其他相关信息。

5. 针对指定json id的PUT方法：
   - HTTP方法：PUT
   - URL格式： `http://{ip address}:{port}/jsons/{json_id}`
   - 描述：该模块用于向服务器上传（或更新）json数据。
   - 功能：客户端可以使用此请求将一个json数据发送到服务器，从而创建新的json数据或更新已有的json数据。

以上五个模块是`web_server`后端开发程序的核心功能，通过这些接口，前端或其他客户端可以与后端进行交互，实现对`pipeline`和`json`数据的管理与操作。请注意，这里的描述仅为示例，实际的API设计可能需要更多的参数和验证措施，以确保数据的安全性和完整性。

## 2 使用方法

在编译程序前，请确认您已使用本项目提供的 `scripts/gen_jsons.py` 脚本，一键生成 `all_files_preview.json` 项目的配置信息索引文件。

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。测试的参数及运行方式是一致的，下面分别以PCIe和SoC模式进行介绍。
### 2.1 PCIe模式编译及运行方法
   ```bash
   mkdir build && cd build
   cmake ..
   make -j
   ./sever
   ```
### 2.2 SoC模式编译及运行方法
   ```bash
   mkdir build && cd build
   cmake ../ -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=/path/to/soc-sdk
   make -j
   ./sever
   ```

>**注意：**

soc环境运行时如果报错，类似问题均需要设置环境变量：
```bash
./yolov5_demo: error while loading shared libraries: libivslogger.so: cannot open shared object file: No such file or directory
```

需要设置环境变量
```bash
export LD_LIBRARY_PATH=path-to/sophon-stream/build/lib/:$LD_LIBRARY_PATH
```

## 3 输入输出样例

### 3.1 获取pipeline List的GET方法：
   - 输入样例：
   无输入
   - 输出样例：
   ```json
   {
      "data": [
         {
            "is_running": false,
            "json_list": [
               {
                  "json_id": "0",
                  "json_name": "decode.json"
               },
               ......
            ],
            "pipeline_id": 0,
            "pipeline_name": "yolov5"
         },
         ......
      ],
      "message": "get pipeline list success",
      "status": "success"
   }
   ```

### 3.2 针对指定pipeline id的GET方法：
  - 输入样例：
   无输入
  - 输出样例：
   ```json
   {
	"data": [
		{
			"is_running": false,
			"json_list": [
				{
					"json_id": "0",
					"json_name": "decode.json"
				},
				......
			],
			"pipeline_id": 0,
			"pipeline_name": "yolov5"
		}
	],
	"message": "get pipeline id content success",
	"status": "success"
   }
   ```

### 3.3 针对指定pipeline id的PATCH方法：
   - 输入样例：
   ```json
   {
    "is_running":true
   }
   ```
   - 输出样例：
   ```json
   {
      "data": [
         {
            "is_running": true,
            "json_list": [],
            "pipeline_id": 0,
            "pipeline_name": "yolov5"
         }
      ],
      "message": "patch pipeline success",
      "status": "success"
   }
   ```

### 3.4 针对指定json id的GET方法：
   - 输入样例：
   无输入
   - 输出样例：
   ```json
   {
   "data": {
      "json_content": {
         "configure": {},
         "name": "decode",
         "shared_object": "../../../build/lib/libdecode.so",
         "side": "sophgo",
         "thread_number": 1
      },
      "json_id": 0,
      "json_name": "decode.json"
   },
   "message": "get json id content success!!!",
   "status": "success"
   }
   ```

### 3.5 针对指定json id的PUT方法：
   - 输入样例：
   无输入
   - 输出样例：
   ```json
   {
	"data": {
		"json_content": {
			"configure": {},
			"name": "decode",
			"shared_object": "../../../build/lib/libdecode.so",
			"side": "sophgo",
			"thread_number": 1
		},
		"json_id": 0,
		"json_name": "decode.json"
	},
	"message": "put json id content success!!!",
	"status": "success"
   }
   ```


