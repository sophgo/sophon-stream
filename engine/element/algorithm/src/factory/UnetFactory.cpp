#include "UnetFactory.h"
#include "../AlgorithmFactorySelector.h"
#include "../inference/UnetInference.h"
#include "../pre_process/UnetPre.h"
#include "../post_process/UnetPost.h"
#include "../context/SophgoContext.h"

namespace sophon_stream{
namespace algorithm{
namespace factory{

/**
 * create a context
 * @return context object
*/
std::shared_ptr<algorithm::Context> UnetFactory::makeContext(){
    return std::static_pointer_cast<algorithm::Context>(std::make_shared<context::SophgoContext>());
}

/**
 * create preprocess
 * @return preprocess object
*/
std::shared_ptr<algorithm::PreProcess> UnetFactory::makePreProcess(){
    return std::static_pointer_cast<algorithm::PreProcess>(std::make_shared<pre_process::UnetPre>());
}

/**
 * create inference 
 * @return inference object
*/
std::shared_ptr<algorithm::Inference> UnetFactory::makeInference(){
    return std::static_pointer_cast<algorithm::Inference>(std::make_shared<inference::UnetInference>());
}

/**
 * create postprocess
 * @return postprocess object
*/
std::shared_ptr<algorithm::PostProcess> UnetFactory::makePostProcess(){
    return std::static_pointer_cast<algorithm::PostProcess>(std::make_shared<post_process::UnetPost>());
}

REGISTER_ALGORITHM_FACTORY("sophgo", "Unet", UnetFactory);
}
}
}
