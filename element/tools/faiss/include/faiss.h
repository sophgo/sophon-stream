//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FAISS_H_
#define SOPHON_STREAM_ELEMENT_FAISS_H_

#include "bmcv_api_ext.h"

//#if BMCV_VERSION_MAJOR <= 1

#include "common/object_metadata.h"
#include "element.h"

extern "C" {
extern bm_status_t bmcv_faiss_indexflatIP(bm_handle_t handle,
                                    bm_device_mem_t input_data_global_addr,
                                    bm_device_mem_t db_data_global_addr,
                                    bm_device_mem_t buffer_global_addr,
                                    bm_device_mem_t output_sorted_similarity_global_addr,
                                    bm_device_mem_t output_sorted_index_global_addr,
                                    int vec_dims,
                                    int query_vecs_num,
                                    int database_vecs_num,
                                    int sort_cnt,
                                    int is_transpose,
                                    int input_dtype,
                                    int output_dtype) __attribute__((weak));
}

namespace sophon_stream {
namespace element {
namespace faiss {
class Faiss : public ::sophon_stream::framework::Element {
 public:
  Faiss();
  ~Faiss() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_DEFAULT_PORT_FILED =
      "default_port";
  static constexpr const char* CONFIG_INTERNAL_DB_DATA_PATH_FILED = "db_path";
  static constexpr const char* CONFIG_INTERNAL_LABEL_PATH_FILED = "label_path";
  int subId = 0;

 private:
  int mDefaultPort;

  int sort_cnt = 5;
  int query_vecs_num = 1;
  int db_vecs_num = 300;
  int is_transpose = 1;
  int input_dtype = 5;
  int output_dtype = 5;
  int vec_dims = 512;

  std::vector<std::string> mClassNames;
  float* db_data;
  float* output_dis;
  int* output_inx;

  void getFaceId(std::shared_ptr<common::RecognizedObjectMetadata> resnetObj);
  bm_handle_t handle = nullptr;
  bm_device_mem_t query_data_dev_mem;
  bm_device_mem_t db_data_dev_mem;
  bm_device_mem_t buffer_dev_mem;
  bm_device_mem_t sorted_similarity_dev_mem;
  bm_device_mem_t sorted_index_dev_mem;
  std::mutex mutex;
  
};

}  // namespace faiss
}  // namespace element
}  // namespace sophon_stream

//#endif
#endif
