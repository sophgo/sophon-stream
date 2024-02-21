//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//


#include "common_tool.h"
#include <stdio.h>

int save_frame_to_yuv(bm_handle_t& handle, AVFrame* frame, const char* filename,
                      bool data_on_device_mem) {
                        
  // stream 无压缩格式，直接退出
  if (frame->channel_layout == 101) return -1;
  
  if (!frame || !filename) {
    IVS_ERROR("Invalid frame or filename.\n");
    return -1;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    IVS_ERROR("Could not open file %s for writing.\n", filename);
    return -1;
  }

  int width = frame->width;
  int height = frame->height;
  unsigned char* buffer = nullptr;
  bm_status_t status;

  if (data_on_device_mem) {
    // 为系统内存分配缓冲区以存储从设备内存拷贝的数据
    // 这里只分配了Y分量的大小，对于UV分量，我们会在下面分别处理
    buffer = new unsigned char[width * height];
    if (!buffer) {
      IVS_ERROR("Could not allocate buffer for image data.\n");
      fclose(file);
      return -1;
    }
  }

  // 写入Y分量
  if (data_on_device_mem) {
    bm_device_mem_t y_mem = bm_mem_from_device(
        (long long unsigned int)frame->data[4], width * height);
    status = bm_memcpy_d2s_partial(handle, buffer, y_mem, width * height);
    if (status != BM_SUCCESS) {
      IVS_ERROR("Error copying Y component from device memory.\n");
      delete[] buffer;
      fclose(file);
      return -1;
    }
    fwrite(buffer, 1, width * height, file);
  } else {
    for (int y = 0; y < height; y++) {
      fwrite(frame->data[0] + y * frame->linesize[0], 1, width, file);
    }
  }

  // 为UV分量重新分配缓冲区大小
  int uvHeight =
      (frame->format == AV_PIX_FMT_YUV420P ||
       frame->format == AV_PIX_FMT_YUVJ420P ||
       frame->format == AV_PIX_FMT_NV12 || frame->format == AV_PIX_FMT_NV21)
          ? height / 2
          : height;
  int uvWidth = (frame->format == AV_PIX_FMT_YUV420P ||
                 frame->format == AV_PIX_FMT_YUVJ420P ||
                 frame->format == AV_PIX_FMT_YUV422P ||
                 frame->format == AV_PIX_FMT_YUVJ422P)
                    ? width / 2
                    : width;
  if (data_on_device_mem) {
    delete[] buffer; 
    buffer = new unsigned char[uvWidth * uvHeight];
    if (!buffer) {
      IVS_ERROR("Could not reallocate buffer for UV component data.\n");
      fclose(file);
      return -1;
    }
  }

  // 根据不同的格式处理UV分量
  switch (frame->format) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
      // 写入U分量
      if (data_on_device_mem) {
        bm_device_mem_t u_mem = bm_mem_from_device(
            (long long unsigned int)frame->data[5], uvWidth * uvHeight);
        status =
            bm_memcpy_d2s_partial(handle, buffer, u_mem, uvWidth * uvHeight);

        if (status != BM_SUCCESS) {
          IVS_ERROR("Error copying U component from device memory.\n");
          delete[] buffer;
          fclose(file);
          return -1;
        }
        fwrite(buffer, 1, uvWidth * uvHeight, file);
      } else {
        for (int y = 0; y < uvHeight; y++) {
          fwrite(frame->data[1] + y * frame->linesize[1], 1, uvWidth, file);
        }
      }
      // 写入V分量
      if (data_on_device_mem) {
        bm_device_mem_t v_mem = bm_mem_from_device(
            (long long unsigned int)frame->data[6], uvWidth * uvHeight);
        status =
            bm_memcpy_d2s_partial(handle, buffer, v_mem, uvWidth * uvHeight);

        if (status != BM_SUCCESS) {
          IVS_ERROR("Error copying V component from device memory.\n");
          delete[] buffer;
          fclose(file);
          return -1;
        }
        fwrite(buffer, 1, uvWidth * uvHeight, file);
      } else {
        for (int y = 0; y < uvHeight; y++) {
          fwrite(frame->data[2] + y * frame->linesize[2], 1, uvWidth, file);
        }
      }
      break;

    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
      // 写入UV分量
      if (data_on_device_mem) {
        bm_device_mem_t uv_mem = bm_mem_from_device(
            (long long unsigned int)frame->data[5], uvWidth * uvHeight * 2);
        status = bm_memcpy_d2s_partial(handle, buffer, uv_mem,
                                       uvWidth * uvHeight * 2);
        if (status != BM_SUCCESS) {
          IVS_ERROR("Error copying UV components from device memory.\n");
          delete[] buffer;
          fclose(file);
          return -1;
        }
        fwrite(buffer, 1, uvWidth * uvHeight * 2, file);
      } else {
        for (int y = 0; y < uvHeight; y++) {
          fwrite(frame->data[1] + y * frame->linesize[1], 1, width,
                 file);  // NV12/NV21 has width-sized UV components
        }
      }
      break;

    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
      // 写入U分量
      if (data_on_device_mem) {
        bm_device_mem_t u_mem = bm_mem_from_device(
            (long long unsigned int)frame->data[5], width * height);
        status = bm_memcpy_d2s_partial(handle, buffer, u_mem, width * height);
        if (status != BM_SUCCESS) {
          IVS_ERROR("Error copying U component from device memory.\n");
          delete[] buffer;
          fclose(file);
          return -1;
        }
        fwrite(buffer, 1, width * height, file);
      } else {
        for (int y = 0; y < height; y++) {
          fwrite(frame->data[1] + y * frame->linesize[1], 1, width, file);
        }
      }
      // 写入V分量
      if (data_on_device_mem) {
        bm_device_mem_t v_mem = bm_mem_from_device(
            (long long unsigned int)frame->data[6], width * height);
        status = bm_memcpy_d2s_partial(handle, buffer, v_mem, width * height);
        if (status != BM_SUCCESS) {
          IVS_ERROR("Error copying V component from device memory.\n");
          delete[] buffer;
          fclose(file);
          return -1;
        }
        fwrite(buffer, 1, width * height, file);
      } else {
        for (int y = 0; y < height; y++) {
          fwrite(frame->data[2] + y * frame->linesize[2], 1, width, file);
        }
      }
      break;

      // 添加其他格式的处理逻辑，如果有的话

    default:
      IVS_ERROR("Unsupported pixel format.\n");
      if (buffer) delete[] buffer;
      fclose(file);
      return -2;
  }

  // 释放缓冲区（如果已分配）
  if (buffer) delete[] buffer;

  fclose(file);
  return 0;
}