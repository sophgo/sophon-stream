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

int save_frame_to_yuv(AVFrame *frame, const char *filename) {
    if (!frame || !filename) {
        IVS_ERROR("Invalid frame or filename.\n");
        return -1;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        IVS_ERROR("Could not open file %s for writing.\n", filename);
        return -1;
    }

    int width = frame->width;
    int height = frame->height;
  

    for (int y = 0; y < height; y++) {
        fwrite(frame->data[0] + y * frame->linesize[0], 1, width, file);
    }

    // The U and V planes are half the height of the Y plane for 420 and 422 formats.
    int uvHeight = (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P ||
                    frame->format == AV_PIX_FMT_NV12 || frame->format == AV_PIX_FMT_NV21) ? height / 2 : height;

    // The U and V planes are half the width of the Y plane for 420 and 422 formats.
    int uvWidth = (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P ||
                   frame->format == AV_PIX_FMT_YUV422P || frame->format == AV_PIX_FMT_YUVJ422P) ? width / 2 : width;

    switch (frame->format) {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
        case AV_PIX_FMT_YUV422P:
        case AV_PIX_FMT_YUVJ422P:
            for (int y = 0; y < uvHeight; y++) {
                fwrite(frame->data[1] + y * frame->linesize[1], 1, uvWidth, file);
                fwrite(frame->data[2] + y * frame->linesize[2], 1, uvWidth, file);
            }
            break;

        case AV_PIX_FMT_NV12:
        case AV_PIX_FMT_NV21:
            for (int y = 0; y < uvHeight; y++) {
                fwrite(frame->data[1] + y * frame->linesize[1], 1, width, file); // NV12/NV21 has width-sized UV components
            }
            break;

        case AV_PIX_FMT_YUV444P:
        case AV_PIX_FMT_YUVJ444P:
            for (int y = 0; y < height; y++) {
                fwrite(frame->data[1] + y * frame->linesize[1], 1, width, file);
                fwrite(frame->data[2] + y * frame->linesize[2], 1, width, file);
            }
            break;

        // Add any additional formats here

        default:
            IVS_ERROR("Unsupported pixel format.\n");
            fclose(file);
            return -2;
    }

    fclose(file);
    return 0;
}
