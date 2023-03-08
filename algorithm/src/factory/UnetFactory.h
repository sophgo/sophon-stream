#pragma once

#include "../AlgorithmFactory.h"
#include "../Context.h"
#include "../Inference.h"
#include "../PostProcess.h"
#include "../PreProcess.h"

namespace sophon_stream{
namespace algorithm{
namespace factory{

// Unet 工厂
class UnetFactory : public algorithm::AlgorithmFactory{
    public:
    /**
     * create context
     * @return context object 
    */
   std::shared_ptr<algorithm::Context> makeContext() override;

   /**
    * create preprocess
    * @return preprocess object
   */
  std::shared_ptr<algorithm::PreProcess> makePreprocess() override;

  /**
   * create inference
   * @return inference object
  */
 std::shared_ptr<algorithm::Inference> makeInference() override;

 /**
  * create postprocess
  * @return postprocess object
 */
std::shared_ptr<algorithm::PostProcess> makePostProcess() override;

    
};
}
}
}