#include "UnetInference.h"
#include <fstream>
#include "../context/SophgoContext.h"

namespace sophgo_stream {
namespace algorithm {
namespace inference {

UnetInference::~UnetInference() {

}
/**
 * init device and engine
 * @param[in] context: model path, inputs and outputs name...
*/
common::ErrorCode UnetInference::init(algorithm::Context& context)
{
    context::SophgoContext * pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
    pSophgoContext->m_outThreshold = 
}


}
}
}