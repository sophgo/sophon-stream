#pragma once

#include "../PreProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

/**
 * face transformations gpu process class
 */
class HostEncodePre : public algorithm::PreProcess {
  public :

    /**
     * preprocess
     * @param[in] context: input and output config
     * @param[in] objectMetadatas: inputData
     * @return preprocess error code or common::ErrorCode::SUCCESS
     */
    common::ErrorCode preProcess(algorithm::Context& context,
                                 common::ObjectMetadatas& objectMetadatas) override;


};

} // namespace pre_process
} // namespace algorithm
} // namespace sophon_stream
