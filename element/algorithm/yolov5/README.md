1.yolov5目标检测算法插件
为了提高pipeline性能，模型前处理、推理、后处理在3个element；通过设置`stage`字段指明element具体是前处理、推理、后处理
BM1684不支持`use_tpu_kernel`为`true`

2.配置
```
{
  "configure":{
      "model_path":"../data/models/yolov5s_tpukernel_int8_4b.bmodel",
      "threshold_conf":0.5,
      "threshold_nms":0.5,
      "stage":["infer"],
      "use_tpu_kernel": true
  },
  "shared_object":"../../../element/algorithm/yolov5/build/libyolov5.so",
  "device_id":0,
  "id":0,
  "name":"yolov5",
  "side":"sophgo",
  "thread_number":1
}
```