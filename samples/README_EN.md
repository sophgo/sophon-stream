# sophon-stream samples instruction

English | [简体中文](README.md)

## 1. Introduction

This directory contains reference demo provided by sophon-stream.

The directory structure is as follows:

```bash
./samples/
├── bytetrack                                       # detection and tracking demo
├── CMakeLists.txt                                  # cmake file
├── include                                         # header files for plotting functions, etc
├── license_plate_recognition                       # vehicle detection and license plate recognition demo
├── openpose                                        # pose estimation demo
├── README.md                                       # user manual
├── README_EN.md                                    # user manual in English
├── resnet                                          # classification demo
├── retinaface                                      # face detection demo
├── retinaface_distributor_resnet_faiss_converger   # face detection and recognition demo
├── src                                             # the only main function
├── yolov5                                          # yolov5 detection demo
├── yolov5_bytetrack_distributor_resnet_converger   # detection, tracking and recognition demo
├── yolox                                           # yolox detection demo
└── yolox_bytetrack_osd_encode                      # detection, tracking, drawing and streaming demo
```

This directory contains multiple routines such as face detection, license plate recognition, and more. Each routine's directory includes configuration files and scripts for downloading models, videos, and other data. For all these routines, they share the same main function, namely `sophon-stream/samples/src/main.cc`. This main function primarily functions to retrieve configuration files based on preset paths, parse input data for the routine, and invoke a unified lower-level interface to configure all elements. It initiates the entire pipeline.

For example，you can run `yolov5_bytetrack_distributor_resnet_converger` routine as follow :

```bash
./main --demo_config_path=../yolov5_bytetrack_distributor_resnet_converger/config/yolov5_bytetrack_distributor_resnet_converger_demo.json
```

Then, the main function navigates to the directory of the `yolov5_bytetrack_distributor_resnet_converger` routine to search for configuration files. It constructs a pipeline based on the configurations specified in these files. Consequently, the running pipeline possesses the functionality of 'detection + tracking + recognition'.

For others，you can run `license_plate_recognition` routine as follow：

```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

Then, the main function will then navigate to the directory of the `license_plate_recognition` routine to search for configuration files. Following the specifications within these configuration files, it constructs a pipeline that incorporates 'vehicle detection + license plate recognition' functionalities.

The benefit of this design is the segregation between users and the underlying code, allowing users to focus solely on configuring JSON files without needing to consider the operational logic of the stream framework.

> Notes
* For detailed information about each routine, please refer to the `README_EN.md` file located within each routine's directory.