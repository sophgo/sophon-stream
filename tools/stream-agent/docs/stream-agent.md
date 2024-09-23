## /stream-agent/创建任务

#### 接口URL
> http://{{host}}/task/create

#### 请求方式
> POST

#### Content-Type
> json

#### 请求Header参数
参数名 | 示例值 | 参数类型 | 是否必填 | 参数描述
--- | --- | --- | --- | ---
Content-Type | application/json;charset=UTF-8 | String | 是 | -
#### 请求Body参数
```javascript
{
  "TaskID": "1001",
  "InputSrc": {
    "SrcID": "test",
    "StreamSrc": {
      "Address": "/opt/sophon/area.mp4"
    }
  },
  "Algorithm": [
    {
      "Type": 2,
      "TrackInterval": 1,
      "DetectInterval": 150,
      "AlarmInterval": 1,
      "threshold": 50,
      "TargetSize": {
        "MinDetect": 30,
        "MaxDetect": 250
      },
      "DetectInfos": [
        {
          "TripWire": {
            "LineStart": {
              "X": 0,
              "Y": 0
            },
            "LineEnd": {
              "X": 0,
              "Y": 0
            },
            "DirectStart": {
              "X": 0,
              "Y": 0
            },
            "DirectEnd": {
              "X": 0,
              "Y": 0
            }
          },
          "HotArea": [
            {
              "X": 0,
              "Y": 0
            },
            {
              "X": 1920,
              "Y": 0
            },
            {
              "X": 1920,
              "Y": 1024
            },
            {
              "X": 0,
              "Y": 1024
            }
          ]
        }
      ]
    }
  ],
  "Reporting": {
    "ReportUrlList": [
      "http://172.28.8.86:8089/api/upload"
    ]
  }
}
```
参数名 | 示例值 | 参数类型 | 是否必填 | 参数描述
--- | --- | --- | --- | ---
TaskID | 1001 | String | 是 | 任务名称
InputSrc | - | Object | 是 | -
InputSrc.SrcID | test | String | 是 | 视频源名称
InputSrc.StreamSrc | - | Object | 是 | -
InputSrc.StreamSrc.Address | /opt/sophon/area.mp4 | String | 是 | 视频地址，视频文件绝对路径或者rtsp流等
Algorithm | - | Array | 是 | -
Algorithm.Type | 2 | Integer | 是 | 算法类型：{1: "tripwire", 2: "license_area_intrusion", 3: "yolov5", 4: "yolov8", 5: "openpose"}
Algorithm.TrackInterval | 1 | Integer | 是 | 追踪间隔
Algorithm.DetectInterval | 150 | Integer | 是 | 检测间隔（每150帧检测一次）
Algorithm.AlarmInterval | 1 | Integer | 是 | 告警间隔
Algorithm.threshold | 50 | Integer | 是 | 阈值（1-100）
Algorithm.TargetSize | - | Object | 是 | -
Algorithm.TargetSize.MinDetect | 30 | Integer | 是 | -
Algorithm.TargetSize.MaxDetect | 250 | Integer | 是 | -
Algorithm.DetectInfos | - | Array | 是 | -
Algorithm.DetectInfos.TripWire | - | Object | 是 | 人员越线检测的检测线
Algorithm.DetectInfos.TripWire.LineStart | - | Object | 是 | -
Algorithm.DetectInfos.TripWire.LineStart.X | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.LineStart.Y | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.LineEnd | - | Object | 是 | -
Algorithm.DetectInfos.TripWire.LineEnd.X | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.LineEnd.Y | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.DirectStart | - | Object | 是 | -
Algorithm.DetectInfos.TripWire.DirectStart.X | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.DirectStart.Y | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.DirectEnd | - | Object | 是 | -
Algorithm.DetectInfos.TripWire.DirectEnd.X | 0 | Integer | 是 | -
Algorithm.DetectInfos.TripWire.DirectEnd.Y | 0 | Integer | 是 | -
Algorithm.DetectInfos.HotArea | - | Array | 是 | 检测区域
Algorithm.DetectInfos.HotArea.X | 0 | Integer | 是 | -
Algorithm.DetectInfos.HotArea.Y | 0 | Integer | 是 | -
Reporting | - | Object | 是 | -
Reporting.ReportUrlList | http://172.28.8.86:8089/api/upload | Array | 是 | 告警上报地址

#### 成功响应示例
```javascript
{
	"Code": 0,
	"Msg": "success"
}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
Code | 0 | Integer | 0表示成功，其他表示失败
Msg | success | String | -
#### 错误响应示例
```javascript
{
    "code": 1,
    "msg": "失败原因"
}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
code | 1 | Integer | -
msg | 失败原因 | String | -
## /stream-agent/查询任务列表


#### 接口URL
> http://{{host}}/task/list

#### 请求方式
> POST

#### Content-Type
> json

#### 请求Header参数
参数名 | 示例值 | 参数类型 | 是否必填 | 参数描述
--- | --- | --- | --- | ---
Content-Type | application/json;charset=UTF-8 | String | 是 | -
#### 请求Body参数
```javascript
{
}
```
#### 认证方式
```text
noauth
```
#### 预执行脚本
```javascript
暂无预执行脚本
```
#### 后执行脚本
```javascript
暂无后执行脚本
```
#### 成功响应示例
```javascript
{"Code":0,"Msg":"success","Result":[{"Status":1,"TaskID":"1001"}]}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
Code | 0 | Integer | 0表示成功，其他表示失败
Msg | success | String | -
Result | - | Array | -
Result.Status | 1 | Integer | 1表示任务运行中，0表示任务停止
Result.TaskID | 1001 | String | 任务名称
#### 错误响应示例
```javascript
{
    "code": 1,
    "msg": "失败原因"
}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
code | 1 | Integer | -
msg | 失败原因 | String | -
## /stream-agent/删除任务
```text
根据开始时间、结束时间、视频通道、算法类型对告警信息进行检索，显示告警图片和告警信息
```
#### 接口状态
> 已完成

#### 接口URL
> http://{{host}}/task/delete

#### 请求方式
> POST

#### Content-Type
> json

#### 请求Header参数
参数名 | 示例值 | 参数类型 | 是否必填 | 参数描述
--- | --- | --- | --- | ---
Content-Type | application/json;charset=UTF-8 | String | 是 | -
#### 请求Body参数
```javascript
{
   "TaskID": "1001"
}
```
参数名 | 示例值 | 参数类型 | 是否必填 | 参数描述
--- | --- | --- | --- | ---
TaskID | 1001 | String | 是 | 任务名称

#### 成功响应示例
```javascript
{"Code":0,"Msg":"success"}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
Code | 0 | Integer | 0表示成功，其他表示失败
Msg | success | String | -
#### 错误响应示例
```javascript
{
    "code": 1,
    "msg": "失败原因"
}
```
参数名 | 示例值 | 参数类型 | 参数描述
--- | --- | --- | ---
code | 1 | Integer | -
msg | 失败原因 | String | -