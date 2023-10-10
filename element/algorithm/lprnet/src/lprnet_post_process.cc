//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lprnet_post_process.h"

#include <cmath>

#include "common/logger.h"
#include <iostream>
#include <string>


namespace sophon_stream {
namespace element {
namespace lprnet {

static char const* arr_chars[] = {
    "京", "沪", "津", "渝", "冀", "晋", "蒙", "辽", "吉", "黑", "苏", "浙",
    "皖", "闽", "赣", "鲁", "豫", "鄂", "湘", "粤", "桂", "琼", "川", "贵",
    "云", "藏", "陕", "甘", "青", "宁", "新", "0",  "1",  "2",  "3",  "4",
    "5",  "6",  "7",  "8",  "9",  "A",  "B",  "C",  "D",  "E",  "F",  "G",
    "H",  "J",  "K",  "L",  "M",  "N",  "P",  "Q",  "R",  "S",  "T",  "U",
    "V",  "W",  "X",  "Y",  "Z",  "I",  "O",  "-"};

int LprnetPostProcess::argmax(float* data, int num) {
    float max_value = -1e10;
    int max_index = 0;
    for (int i = 0; i < num; ++i) {
        float value = data[i];
        if (value > max_value) {
            max_value = value;
            max_index = i;
        }
    }
    return max_index;
}

std::string LprnetPostProcess::get_res(int pred_num[], int len_char, int clas_char) {
    int no_repeat_blank[20];
    int cn_no_repeat_blank = 0;
    int pre_c = pred_num[0];
    if (pre_c != clas_char - 1) {
        no_repeat_blank[0] = pre_c;
        cn_no_repeat_blank++;
    }
    for (int i = 0; i < len_char; i++) {
        if (pred_num[i] == pre_c)
            continue;
        if (pred_num[i] == clas_char - 1) {
            pre_c = pred_num[i];
            continue;
        }
        no_repeat_blank[cn_no_repeat_blank] = pred_num[i];
        pre_c = pred_num[i];
        cn_no_repeat_blank++;
    }

    std::string res = "";
    for (int j = 0; j < cn_no_repeat_blank; j++) {
        res = res + arr_chars[no_repeat_blank[j]];
    }

    return res;
}


void LprnetPostProcess::init(std::shared_ptr<LprnetContext> context) {}

void LprnetPostProcess::postProcess(std::shared_ptr<LprnetContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
    if (objectMetadatas.size() == 0) return;
    
    int idx = 0;  
    // get 1 batch data 
    for (auto obj : objectMetadatas) {
        // stream end control 
        if (obj->mFrame->mEndOfStream) break;

        // init output tensors 
        std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
        for (int i = 0; i < context->output_num; i++) {
        outputTensors[i] = std::make_shared<BMNNTensor>(
            obj->mOutputBMtensors->handle,
            context->bmNetwork->m_netinfo->output_names[i],
            context->bmNetwork->m_netinfo->output_scales[i],
            obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
        }

        float* output_data = nullptr;
        float ptr[context->clas_char];
        int pred_num[context->len_char];    
        
        for (int i = 0; i < context->output_num; i++) {
        auto out_tensor = outputTensors[i];
        output_data =
            (float*)out_tensor->get_cpu_data() + idx * context->len_char * context->clas_char;
        for (int j = 0; j < context->len_char; j++) {
            for (int k = 0; k < context->clas_char; k++) {
                ptr[k] = *(output_data + k * context->len_char + j);
            }
            int class_id = argmax(&ptr[0], context->clas_char);
            float confidence = ptr[class_id];
            pred_num[j] = class_id;      
        }
        std::string res = get_res(pred_num, context->len_char, context->clas_char);
        std::shared_ptr<common::RecognizedObjectMetadata> detData =
                  std::make_shared<common::RecognizedObjectMetadata>();   
        detData->mLabelName = res;
        obj->mRecognizedObjectMetadatas.push_back(detData);

        std::cout << res << std::endl;
        if (res.empty()){
            IVS_WARN("License plate recognition came up empty");
        }else{
            IVS_DEBUG("License plate recognizition SUCCESS");
        }
    }
    ++idx;
  }
}

}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream