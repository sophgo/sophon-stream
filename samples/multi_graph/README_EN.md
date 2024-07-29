# Multi-Graph Demo

English | [简体中文](README.md)

## 目录
- [Multi-Graph Demo](#Multi-Graph demo)
  - [Catalogs](#catalogs)
  - [1. Introduction](#1-introduction)
  - [2. Features](#2-features)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Execute](#4-execute)

## 1. Introduction

This routine is used to illustrate how to build a multi-graph application using sophon-stream.

Generally speaking, all input streams on one graph of sophon-stream only support executing the same algorithm flow on the same device. Therefore, when encountering different code streams need to do different calculations, or different code streams run on different devices, you need to use multi-graph to configure it.

Here is a simple example, the Engine contains two Graphs: Graph0 is for decoding-detecting-encoding RTSP; Graph1 is for decoding-detecting-encoding local video files.

The plug-in connection of this routine is shown in the following figure

![multi_graph](./pics/multi_graph.jpg)

## 2. Feature

* Supports BM1684X, BM1684(x86 PCIe、SoC), supports BM1688(SoC).
* Supports tpu_kernel post-process on BM1684X.
* Supports multiple video streams.
* Supports multi-threading.

## 3. Prepare Models and Data

This sample uses data from the yolov5 sample, see [yolov5](../yolov5/README.md) to download the data

## 4. Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../multi_graph/config/multi_graph_demo.json
```
