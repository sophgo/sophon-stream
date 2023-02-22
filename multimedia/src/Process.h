#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Context.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace multimedia {

/**
 * inference base class
 */
class Process {
  public:
    /**
     * init format or codec
     * @param[in] @param[in] context: inputs and outputs name...
     * return
     */
    virtual common::ErrorCode init(multimedia::Context& context) = 0;

    /**
     * process output
     * @param[in] context: inputData and outputDatt
     */
    virtual common::ErrorCode process(multimedia::Context& context,std::shared_ptr<common::ObjectMetadata>&) = 0;

    virtual void uninit() = 0;


};

} // namespace multimedia
} // namespace sophon_stream

