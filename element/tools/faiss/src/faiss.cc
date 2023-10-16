//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "faiss.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace faiss {
Faiss::Faiss() {}
Faiss::~Faiss() {
  delete[] db_data;
  delete[] output_dis;
  delete[] output_inx;
  bm_free_device(handle, query_data_dev_mem);
  bm_free_device(handle, db_data_dev_mem);
  bm_free_device(handle, buffer_dev_mem);
  bm_free_device(handle, sorted_similarity_dev_mem);
  bm_free_device(handle, sorted_index_dev_mem);
}

common::ErrorCode Faiss::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto db_data_path =
        configure.find(CONFIG_INTERNAL_DB_DATA_PATH_FILED)->get<std::string>();

    std::vector<float> db_vec(0);
    std::ifstream db_data_file;
    db_data_file.open(db_data_path);
    assert(db_data_file.is_open());
    // 读取文件数据到数组
    if (db_data_file) {
      std::string line;
      int row_count = 0;
      while (std::getline(db_data_file, line)) {
        std::vector<float> row;
        std::stringstream ss(line);
        float val;
        int col_count = 0;
        while (ss >> val) {
          db_vec.push_back(val);
          col_count++;
        }
        if (row_count == 0) {
          vec_dims = col_count;
        }
        row_count++;
      }
      db_vecs_num = row_count;
    }
    db_data_file.close();

    auto label_path =
        configure.find(CONFIG_INTERNAL_LABEL_PATH_FILED)->get<std::string>();
    std::ifstream istream;
    istream.open(label_path);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length());
      mClassNames.push_back(line);
    }
    istream.close();

    db_data = new float[db_vecs_num * vec_dims];
    output_dis = new float[query_vecs_num * sort_cnt];
    output_inx = new int[query_vecs_num * sort_cnt];
    std::memcpy(db_data, db_vec.data(), db_vec.size() * sizeof(float));

    bm_dev_request(&handle, 0);
    bm_memcpy_s2d(handle, db_data_dev_mem, db_data);
    bm_malloc_device_byte(handle, &buffer_dev_mem,
                          query_vecs_num * db_vecs_num * sizeof(float));
    bm_malloc_device_byte(handle, &sorted_similarity_dev_mem,
                          query_vecs_num * sort_cnt * sizeof(float));
    bm_malloc_device_byte(handle, &sorted_index_dev_mem,
                          query_vecs_num * sort_cnt * sizeof(int));
    bm_malloc_device_byte(handle, &query_data_dev_mem,
                          query_vecs_num * vec_dims * sizeof(float));
    bm_malloc_device_byte(handle, &db_data_dev_mem,
                          db_vecs_num * vec_dims * sizeof(float));

  } while (false);
  return errorCode;
}

void Faiss::getFaceId(
    std::shared_ptr<common::RecognizedObjectMetadata> resnetObj) {
  if (resnetObj != nullptr) {
    // float *input_data = new float[query_vecs_num * vec_dims];
    float* input_data = resnetObj->feature_vector.get();
    for (int i = 0; i < 10; i++) {
      std::cout << input_data[i] << " ";
    }
    // 将 input_data 的值赋给 feature_vector
    // bm_handle_t handle=obj->mFrame->mHandle;
   {
    std::lock_guard<std::mutex> lock(mutex);
    bm_memcpy_s2d(handle, query_data_dev_mem, input_data);
    bmcv_faiss_indexflatIP(handle, query_data_dev_mem, db_data_dev_mem,
                           buffer_dev_mem, sorted_similarity_dev_mem,
                           sorted_index_dev_mem, vec_dims, query_vecs_num,
                           db_vecs_num, sort_cnt, is_transpose, input_dtype,
                           output_dtype);

    bm_memcpy_d2s(handle, output_dis, sorted_similarity_dev_mem);
    bm_memcpy_d2s(handle, output_inx, sorted_index_dev_mem);
   }
    int label_index = output_inx[0];
    resnetObj->mLabelName = mClassNames[label_index];
    resnetObj->mTopKLabels.push_back(output_inx[0]);
  }
}

common::ErrorCode Faiss::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
  int subId = 0;
  // 从resnet取出一个objectMetadata
  // 从data里面取人脸，对一个个人脸做处理，首先需要取出
  for (auto resnetObj : objectMetadata->mRecognizedObjectMetadatas) {
    // mRecognizedObjectMetadatas是一个数组，每个数组包含人脸的框、特征等等,需要做的只是提取特征，填充label
    Faiss::getFaceId(resnetObj);
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  errorCode = pushOutputData(outputPort, outDataPipeId,
                             std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  return errorCode;
}

REGISTER_WORKER("faiss", Faiss)
}  // namespace faiss
}  // namespace element
}  // namespace sophon_stream