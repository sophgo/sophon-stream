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

#include "common/object_metadata.h"
#include "element.h"

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

  void getFaceId(std::shared_ptr<common::RecognizedObjectMetadata> resnetObj);
  bm_handle_t handle = nullptr;
  bm_device_mem_t db_data_dev_mem;
  
};

}  // namespace faiss
}  // namespace element
}  // namespace sophon_stream

#endif