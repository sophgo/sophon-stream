
#include "Yolov5Inference.h"
#include <fstream>
#include "../context/SophgoContext.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

Yolov5Inference::~Yolov5Inference() {

}

/**
 * init device and engine
 * @param[in] @param[in] context: model path,inputs and outputs name...
 */
common::ErrorCode Yolov5Inference::init(algorithm::Context& context) {
    context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
    float confThresh;
    float nmsThresh;
    std::string coco_names_file;
    pSophgoContext->m_confThreshold= confThresh;
    pSophgoContext->m_nmsThreshold = nmsThresh;
    std::ifstream ifs(coco_names_file);
    if (ifs.is_open()) {
      std::string line;
      while(std::getline(ifs, line)) {
        line = line.substr(0, line.length() - 1);
        pSophgoContext->m_class_names.push_back(line);
      }
    }

    //1. get network
    pSophgoContext->m_bmNetwork = pSophgoContext->m_bmContext->network(0);
    
    //2. get input
    pSophgoContext->max_batch = pSophgoContext->m_bmNetwork->maxBatch();
    auto tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
    pSophgoContext->m_net_h = tensor->get_shape()->dims[2];
    pSophgoContext->m_net_w = tensor->get_shape()->dims[3];

    //3. get output
    pSophgoContext->output_num = pSophgoContext->m_bmNetwork->outputTensorNum();
    assert(pSophgoContext->output_num > 0);
    pSophgoContext->min_dim = pSophgoContext->m_bmNetwork->outputTensor(0)->get_shape()->num_dims;

    //4. initialize bmimages
    pSophgoContext->m_resized_imgs.resize(pSophgoContext->max_batch);
    pSophgoContext->m_converto_imgs.resize(pSophgoContext->max_batch);
    // some API only accept bm_image whose stride is aligned to 64
    int aligned_net_w = FFALIGN(pSophgoContext->m_net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    for(int i=0; i<pSophgoContext->max_batch; i++){
      auto ret= bm_image_create(pSophgoContext->m_bmContext->handle(), pSophgoContext->m_net_h, 
      pSophgoContext->m_net_w, 
      FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &pSophgoContext->m_resized_imgs[i], strides);
      assert(BM_SUCCESS == ret);
    }
    bm_image_alloc_contiguous_mem(pSophgoContext->max_batch, pSophgoContext->m_resized_imgs.data());
    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    if (tensor->get_dtype() == BM_INT8){
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    }
    auto ret = bm_image_create_batch(pSophgoContext->m_bmContext->handle(), pSophgoContext->m_net_h, 
    pSophgoContext->m_net_w, FORMAT_RGB_PLANAR, img_dtype, pSophgoContext->m_converto_imgs.data(), pSophgoContext->max_batch);
    assert(BM_SUCCESS == ret);

    // 5.converto
    float input_scale = tensor->get_scale();
    input_scale = input_scale * 1.0 / 255.f;
    pSophgoContext->converto_attr.alpha_0 = input_scale;
    pSophgoContext->converto_attr.beta_0 = 0;
    pSophgoContext->converto_attr.alpha_1 = input_scale;
    pSophgoContext->converto_attr.beta_1 = 0;
    pSophgoContext->converto_attr.alpha_2 = input_scale;
    pSophgoContext->converto_attr.beta_2 = 0;
    return common::ErrorCode::SUCCESS;
}


/**
 * network predict output
 * @param[in] context: inputData and outputDat
 */
common::ErrorCode Yolov5Inference::predict(algorithm::Context& context) {
  context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
  int ret = pSophgoContext->m_bmNetwork->forward();
  return static_cast<common::ErrorCode>(ret);
}

void Yolov5Inference::uninit() {
}

static void* function(){
  return nullptr;
}
static void* fuck(void){
  return nullptr;
}

} // namespace inference
} // namespace algorithm
} // namespace sophon_stream
