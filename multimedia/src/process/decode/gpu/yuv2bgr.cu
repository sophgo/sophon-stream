/**
 *  Copyright (c) 2017 LGPL, Inc. All Rights Reserved
 *  @author Chen Qian (chinahbcq@qq.com)
 *  @date 2017.04.22 14:32:13
 *  @brief gpu颜色空间转换
 */
#include <stdio.h>
// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>
#include <cuda_profiler_api.h>
#include <curand.h>

__global__ void
cvtNV12_BGR(unsigned char *A,
            unsigned char *B,
            const int height,
            const int width,
            const int linesize) {
    int IDX = blockDim.x * blockIdx.x + threadIdx.x;
    long len = width * height;
    if (IDX < len) {
        int j = IDX % width;
        int i = (IDX - j) / width;

        int bgr[3];
        int yIdx, uvIdx, idx;
        int y, u, v;

        yIdx = i * linesize + j;
        uvIdx = linesize * height + (i / 2) * linesize + j - j % 2;

        y = A[yIdx];
        u = A[uvIdx];
        v = A[uvIdx + 1];

        bgr[0] = y + 1.772 * (u - 128);
        bgr[1] = y - 0.34414 * (u - 128) - 0.71414 * (v - 128);
        bgr[2] = y + 1.402 * (v - 128);

        for (int k = 0; k < 3; k++) {
            idx = (i * width + j) * 3 + k;
            if (bgr[k] >= 0 && bgr[k] < 255) {
                B[idx] = bgr[k];
            } else {
                B[idx] = bgr[k] < 0 ? 0 : 255;
            }
        }

    }
}

//R = Y + 1.402 (V - 128)

//G = Y - 0.34414 (U - 128) - 0.71414 (V - 128)

//B = Y + 1.772 (U - 128)

int cvtColor(
    unsigned char *d_req,
    unsigned char *d_res,
    int resolution,
    int height,
    int width,
    int linesize) {

    int threadsPerBlock = 256;
    int blocksPerGrid = (resolution + threadsPerBlock - 1) / threadsPerBlock;
    cvtNV12_BGR <<< blocksPerGrid, threadsPerBlock>>>(d_req, d_res, height, width, linesize);

    return 0;
}
