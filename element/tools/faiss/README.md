# sophon-stream faiss element

sophon-stream faiss element是sophon-stream框架中的一个插件，是一个专用作计算查询向量与数据库向量的内积距离的工具。

## 1. 特性
* 该接口用于 Faiss::IndexFlatIP.search(), 在 BM1684X 上实现。考虑 BM1684X 上 TPU 的连续内存, 针对 100W 底库, 可以在单芯片上一次查询最多约 512 个 256 维的输入。

## 2. 配置参数
sophon-stream faiss插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "db_path":"../data/face_data/faiss_db_data.txt",
        "label_path":"../data/face_data/faiss_index_label.name"
    },
    "shared_object": "../../../build/lib/libfaiss.so",
    "name": "faiss",
    "side": "sophgo",
    "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                                     | 说明               |
| ------------- | ------ | ------------------------------------------ | ------------------ |
| shared_object | string | "../../../build/lib/libfaiss.so"           | libfaiss动态库路径 |
| name          | string | "faiss"                                    | element名称        |
| side          | string | "sophgo"                                   | 设备类型           |
| db_path       | int    | "../data/face_data/faiss_db_data.txt"      | 数据库地址         |
| label_path    | string | "../data/face_data/faiss_index_label.name" | 数据库人脸标签     |

