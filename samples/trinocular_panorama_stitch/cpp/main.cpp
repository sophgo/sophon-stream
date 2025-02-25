#include <iostream>
#include <fstream>
#include <cstdio>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "bmcv_api_ext.h"
#include "opencv2/opencv.hpp"
#include "bmlib_runtime.h"
#include "bmcv_api.h"

#define ALIGN(x, a)      (((x) + ((a)-1)) & ~((a)-1))

void bm_dmem_read_yuv(bm_handle_t handle, bm_device_mem_t* dmem, const char *input_name, unsigned int size)
{
  if (access(input_name, F_OK) != 0 || strlen(input_name) == 0 || 0 >= size)
  {
    return;
  }

  char* input_ptr = (char *)malloc(size);
  FILE *fp_src = fopen(input_name, "rb+");
  int read_size = fread((void *)input_ptr, 1, size, fp_src);
  if (read_size < (unsigned int)size){
      printf("file size is less than %d required bytes\n", size);
  };
  fclose(fp_src);

  if (BM_SUCCESS != bm_malloc_device_byte(handle, dmem, size)){
    printf("bm_malloc_device_byte failed\n");
  }
  if (BM_SUCCESS != bm_memcpy_s2d_partial(handle, *dmem, input_ptr, size)){
    printf("bm_memcpy_s2d failed\n");
  }
  free(input_ptr);
  return;
}

void load_ldc_attr(bmcv_gdc_attr& ldc_attr, char* buffer, std::string grid_info_path){
    FILE* fp = fopen(grid_info_path.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    int grid_size = ftell(fp);
    buffer = (char*)malloc(grid_size);
    memset(buffer, 0, grid_size);
    rewind(fp);
    fread(buffer, grid_size, 1, fp);
    fclose(fp);
    ldc_attr.grid_info.u.system.system_addr = (void*)buffer;
    ldc_attr.grid_info.size = grid_size;
}

void dump_yuv(bm_handle_t handle, std::string filename, bm_image frame){
 
    bm_device_mem_t mems[3];
    bm_image_get_device_mem(frame, mems);
    int size0 = frame.height * frame.width;
    int size1 = size0/4;
    int size2 = size0/4;
     
    char* buffer0 = new char[size0];
    char* buffer1 = new char[size1];
    char* buffer2 = new char[size2];
 
    bm_memcpy_d2s(handle, buffer0, mems[0]);
    bm_memcpy_d2s(handle, buffer1, mems[1]);
    bm_memcpy_d2s(handle, buffer2, mems[2]);
 
    std::ofstream outFile(filename);
    outFile.write(buffer0, size0);
    outFile.write(buffer1, size1);
    outFile.write(buffer2, size2);
    delete[] buffer0;
    delete[] buffer1;
    delete[] buffer2;
}

void dump_rgb(bm_handle_t handle, std::string filename, bm_image frame){
    bm_device_mem_t mems[3];
    bm_image_get_device_mem(frame, mems);
    int size0 = frame.height * frame.width * 3;
     
    uint8_t* buffer0 = new uint8_t[size0];
 
    bm_memcpy_d2s(handle, buffer0, mems[0]);
 
    std::ofstream outFile(filename);
    outFile.write((char*)buffer0, size0);
    delete[] buffer0;
}

bm_image convert_fmt(bm_handle_t handle, bm_image input, bm_image_format_ext_ fmt){
    if(input.image_format == fmt){
        return input;
    }
    int ret = 0;
    bm_image output;
    ret = bm_image_create(handle, input.height, input.width, fmt, input.data_type, &output);
    assert(ret == BM_SUCCESS);
    ret = bm_image_alloc_dev_mem(output, 1);
    assert(ret == BM_SUCCESS);
    ret = bmcv_image_storage_convert(handle, 1, &input, &output);
    assert(ret == BM_SUCCESS);
    return output;
}

bm_image dwa_gdc(bm_handle_t handle, bm_image input, bmcv_gdc_attr ldc_attr, int dwa_dst_h, int dwa_dst_w){
    int ret = 0;
    bm_image_format_ext_ src_fmt = FORMAT_YUV420P;
    bm_image input_, output_;
    if(input.image_format != src_fmt){
        ret = bm_image_create(handle, input.height, input.width, src_fmt, input.data_type, &input_, NULL);
        assert(ret == BM_SUCCESS);
        ret = bm_image_alloc_dev_mem(input_, 1);
        assert(ret == BM_SUCCESS);
        ret = bmcv_image_storage_convert(handle, 1, &input, &input_);
    }else{
        input_ = input;
    }
    ret = bm_image_create(handle, dwa_dst_h, dwa_dst_w, src_fmt, DATA_TYPE_EXT_1N_BYTE, &output_, NULL);
    assert(ret == BM_SUCCESS);
    ret = bm_image_alloc_dev_mem(output_, 1);
    assert(ret == BM_SUCCESS);
    ret = bmcv_dwa_gdc(handle, input_, output_, ldc_attr);
    assert(ret == BM_SUCCESS);
    return output_;
}

void blend_HandleSig(int signum)
{
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  printf("signal happen  %d \n",signum);
  exit(-1);
}

int main(){
    int ret = 0;
    std::string left_img_path = "../datasets/0106/left/left_420.jpg";
    std::string right_img_path = "../datasets/0106/right/right_420.jpg";
    std::string down_img_path = "../datasets/0106/down/down_420.jpg";

    cv::Mat left_img_mat = cv::imread(left_img_path);
    cv::Mat right_img_mat = cv::imread(right_img_path);
    cv::Mat down_img_mat = cv::imread(down_img_path);
    
    bm_image left_img_bmi; cv::bmcv::toBMI(left_img_mat, &left_img_bmi);
    bm_image right_img_bmi; cv::bmcv::toBMI(right_img_mat, &right_img_bmi);
    bm_image down_img_bmi; cv::bmcv::toBMI(down_img_mat, &down_img_bmi);
    
    // gridinfo:
    bmcv_gdc_attr ldc_attr_left = {0};
    char* ldc_buffer_left = NULL;
    bmcv_gdc_attr ldc_attr_right = {0};
    char* ldc_buffer_right = NULL;
    bmcv_gdc_attr ldc_attr_down = {0};
    char* ldc_buffer_down = NULL;
    load_ldc_attr(ldc_attr_left, ldc_buffer_left, "../gridinfo/left_grid_info_bev_90_90_5078_90_90_dst_2880x2880_src_2560x2160.1.dat");
    load_ldc_attr(ldc_attr_right, ldc_buffer_right, "../gridinfo/right_grid_info_bev_90_90_4233_90_90_dst_2880x2880_src_2560x1440.1.dat");
    load_ldc_attr(ldc_attr_down, ldc_buffer_down, "../gridinfo/down_grid_info_bev_90_90_4589_90_90_dst_2880x2880_src_2560x1440.1.dat");

    // dwa:
    int dst_h = 2880;
    int dst_w = 2880;
    bm_handle_t handle;
    ret = bm_dev_request(&handle, 0);
    assert(ret == BM_SUCCESS);
    bm_image left_img_bmi_dwa = dwa_gdc(handle, left_img_bmi, ldc_attr_left, dst_h, dst_w);
    bm_image right_img_bmi_dwa = dwa_gdc(handle, right_img_bmi, ldc_attr_right, dst_h, dst_w);
    bm_image down_img_bmi_dwa = dwa_gdc(handle, down_img_bmi, ldc_attr_down, dst_h, dst_w);
    
    //mask
    std::string left_mask_path = "../datasets/mask/left.bin";
    std::string right_mask_path = "../datasets/mask/right.bin";
    std::string up_mask_path = "../datasets/mask/up.bin";
    std::string down_mask_path = "../datasets/mask/down.bin";

    cv::Mat left_dwa_mat; 
    cv::bmcv::toMAT(&left_img_bmi_dwa, left_dwa_mat);
    cv::imwrite("left_dwa_mat.jpg", left_dwa_mat);
    cv::Mat right_dwa_mat; 
    cv::bmcv::toMAT(&right_img_bmi_dwa, right_dwa_mat);
    cv::imwrite("right_dwa_mat.jpg", right_dwa_mat);
    cv::Mat down_dwa_mat; 
    cv::bmcv::toMAT(&down_img_bmi_dwa, down_dwa_mat);
    cv::imwrite("down_dwa_mat.jpg", down_dwa_mat);


    bm_image blend_input[3] = {left_img_bmi_dwa, right_img_bmi_dwa, down_img_bmi_dwa};
    bm_image blend_output;
    ret = bm_image_create(handle, dst_h, dst_w, blend_input[0].image_format, blend_input[0].data_type, &blend_output);
    assert(ret == BM_SUCCESS);
    ret = bm_image_alloc_dev_mem(blend_output, 1);
    assert(ret == BM_SUCCESS);
    stitch_param stitch_config;
    memset(&stitch_config, 0, sizeof(stitch_config));
    stitch_config.wgt_mode = BM_STITCH_WGT_YUV_SHARE;
    signal(SIGINT, blend_HandleSig);
    signal(SIGTERM, blend_HandleSig);
    stitch_config.ovlap_attr.ovlp_lx[0] = 0;
    stitch_config.ovlap_attr.ovlp_lx[1] = 0;
    stitch_config.ovlap_attr.ovlp_rx[0] = dst_w - 1;
    stitch_config.ovlap_attr.ovlp_rx[1] = dst_w - 1;
    int wgtWidth = ALIGN(dst_w, 16);
    int wgtHeight = dst_h;

    int wgt_len = wgtWidth * wgtHeight;
    if (stitch_config.wgt_mode == BM_STITCH_WGT_UV_SHARE)
        wgt_len = wgt_len << 1;
    bm_dmem_read_yuv(handle, &stitch_config.wgt_phy_mem[0][0], left_mask_path.c_str(), wgt_len);
    bm_dmem_read_yuv(handle, &stitch_config.wgt_phy_mem[0][1], right_mask_path.c_str(), wgt_len);
    bm_dmem_read_yuv(handle, &stitch_config.wgt_phy_mem[1][0], up_mask_path.c_str(), wgt_len);
    bm_dmem_read_yuv(handle, &stitch_config.wgt_phy_mem[1][1], down_mask_path.c_str(), wgt_len);

    ret = bmcv_blending(handle, 3, blend_input, blend_output, stitch_config);

    cv::Mat blend_output_mat; 
    cv::bmcv::toMAT(&blend_output, blend_output_mat);
    cv::imwrite("blend_output.jpg", blend_output_mat);

    bm_image_destroy(left_img_bmi);    
    bm_image_destroy(right_img_bmi);    
    bm_image_destroy(down_img_bmi);
    bm_image_destroy(left_img_bmi_dwa);    
    bm_image_destroy(right_img_bmi_dwa);    
    bm_image_destroy(down_img_bmi_dwa);    
    bm_image_destroy(blend_output);    
    for(int i = 0; i < 2; i++){
        bm_free_device(handle, stitch_config.wgt_phy_mem[i][0]);
        bm_free_device(handle, stitch_config.wgt_phy_mem[i][1]);
    }
    free(ldc_buffer_left);
    free(ldc_buffer_right);
    free(ldc_buffer_down);
    return 0;
}