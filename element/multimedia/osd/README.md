1.osd插件，负责算法结果的可视化，支持目标检测、目标跟踪算法结果可视化

2.配置
{
  "configure": {
    "osd_type": "track",
    "class_names": "../data/coco.names"
  },
  "shared_object": "../../../build/lib/libosd.so",
  "device_id": 0,
  "id": 0,
  "name": "osd",
  "side": "sophgo",
  "thread_number": 1
}