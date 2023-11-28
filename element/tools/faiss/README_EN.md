# sophon-stream faiss element

English | [简体中文](README.md)

sophon-stream faiss element is a plugin within the Sophon Stream framework designed specifically for computing the inner-product distance between query vectors and database vectors.

## 1. Feature
* This interface is utilized for `Faiss::IndexFlatIP.search()` and is implemented on BM1684X. Considering the continuous memory of the TPU on BM1684X, for a database of 1 million entries, it's feasible to query a maximum of around 512 sets of 256-dimensional inputs on a single processor at a time.

## 2. Configuration Parameters
Sophon-stream Faiss plugin comes with several configurable parameters that can be adjusted according to requirements. Here are some commonly used parameters:
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

| Parameter Name|  name  |        Default value                       |            Description           |
| ------------- | ------ | ------------------------------------------ | ------------------ |
| shared_object | string | "../../../build/lib/libfaiss.so"           | libfaiss dynamic library path |
| name          | string | "faiss"                                    | element name        |
| side          | string | "sophgo"                                   | device type           |
| db_path       | int    | "../data/face_data/faiss_db_data.txt"      | database address       |
| label_path    | string | "../data/face_data/faiss_index_label.name" | face labels     |

