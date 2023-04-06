#include "Yolov5Post.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

void Yolov5Post::init(algorithm::Context& context) {
}

int Yolov5Post::argmax(float* data, int num){
  float max_value = 0.0;
  int max_index = 0;
  for(int i = 0; i < num; ++i) {
    float value = data[i];
    if (value > max_value) {
      max_value = value;
      max_index = i;
    }
  }

  return max_index;
}

float Yolov5Post::sigmoid(float x){
  return 1.0 / (1 + expf(-x));
}

void Yolov5Post::NMS(YoloV5BoxVec &dets, float nmsConfidence)
{
  int length = dets.size();
  int index = length - 1;

  std::sort(dets.begin(), dets.end(), [](const YoloV5Box& a, const YoloV5Box& b) {
      return a.score < b.score;
      });

  std::vector<float> areas(length);
  for (int i=0; i<length; i++)
  {
    areas[i] = dets[i].width * dets[i].height;
  }

  while (index  > 0)
  {
    int i = 0;
    while (i < index)
    {
      float left    = std::max(dets[index].x,   dets[i].x);
      float top     = std::max(dets[index].y,    dets[i].y);
      float right   = std::min(dets[index].x + dets[index].width,  dets[i].x + dets[i].width);
      float bottom  = std::min(dets[index].y + dets[index].height, dets[i].y + dets[i].height);
      float overlap = std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
      if (overlap / (areas[index] + areas[i] - overlap) > nmsConfidence)
      {
        areas.erase(areas.begin() + i);
        dets.erase(dets.begin() + i);
        index --;
      }
      else
      {
        i++;
      }
    }
    index--;
  }
}


const std::vector<std::vector<int>> colors = {{255, 0, 0}, {255, 85, 0}, {255, 170, 0}, {255, 255, 0}, {170, 255, 0}, \
                {85, 255, 0}, {0, 255, 0}, {0, 255, 85}, {0, 255, 170}, {0, 255, 255}, {0, 170, 255}, {0, 85, 255}, \
                {0, 0, 255}, {85, 0, 255}, {170, 0, 255}, {255, 0, 255}, {255, 0, 170}, {255, 0, 85}, {255, 0, 0},\
                {255, 0, 255}, {255, 85, 255}, {255, 170, 255}, {255, 255, 255}, {170, 255, 255}, {85, 255, 255}};

void draw_bmcv(bm_handle_t &&handle, int classId, float conf, int left, int top, int width, int height, bm_image& frame)   // Draw the predicted bounding box
{
  int colors_num = colors.size();
  //Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << "," << rect.crop_h << std::endl;
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, colors[classId % colors_num][0], colors[classId % colors_num][1], colors[classId % colors_num][2]);
  // cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 3);


}

void Yolov5Post::postProcess(algorithm::Context& context,
        common::ObjectMetadatas& objectMetadatas) {
            context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
             YoloV5BoxVec yolobox_vec;
            std::vector<cv::Rect> bbox_vec;
            std::vector<std::shared_ptr<BMNNTensor>> outputTensors(pSophgoContext->output_num);
            for(int i=0; i<pSophgoContext->output_num; i++){
                outputTensors[i] = pSophgoContext->m_bmNetwork->outputTensor(i);
            }
            
            for(int batch_idx = 0; batch_idx < pSophgoContext->max_batch; ++ batch_idx)
            {
                yolobox_vec.clear();
                if(pSophgoContext->mEndOfStream) continue;
                int frame_width = pSophgoContext->m_frame_w;
                int frame_height = pSophgoContext->m_frame_h;

                int tx1 = 0, ty1 = 0;
            #ifdef USE_ASPECT_RATIO
                bool isAlignWidth = false;
                float ratio = context::get_aspect_scaled_ratio(frame_width, frame_height, 
                pSophgoContext->m_net_w, pSophgoContext->m_net_h, &isAlignWidth);
                if (isAlignWidth) {
                ty1 = (int)((pSophgoContext->m_net_h - (int)(frame_height*ratio)) / 2);
                }else{
                tx1 = (int)((pSophgoContext->m_net_w - (int)(frame_width*ratio)) / 2);
                }
            #endif

                int min_idx = 0;
                int box_num = 0;
                for(int i=0; i<pSophgoContext->output_num; i++){
                auto output_shape = pSophgoContext->m_bmNetwork->outputTensor(i)->get_shape();
                auto output_dims = output_shape->num_dims;
                assert(output_dims == 3 || output_dims == 5);
                if(output_dims == 5){
                    box_num += output_shape->dims[1] * output_shape->dims[2] * output_shape->dims[3];
                }

                if(pSophgoContext->min_dim>output_dims){
                    min_idx = i;
                    pSophgoContext->min_dim = output_dims;
                }
                }

                auto out_tensor = outputTensors[min_idx];
                int nout = out_tensor->get_shape()->dims[pSophgoContext->min_dim-1];
                pSophgoContext->m_class_num = nout - 5;

                float* output_data = nullptr;
                std::vector<float> decoded_data;

                if(pSophgoContext->min_dim ==3 && pSophgoContext->output_num !=1){
                std::cout<<"--> WARNING: the current bmodel has redundant outputs"<<std::endl;
                std::cout<<"             you can remove the redundant outputs to improve performance"<< std::endl;
                std::cout<<std::endl;
                }

                if(pSophgoContext->min_dim == 5){
                // std::cout<<"--> Note: Decoding Boxes"<<std::endl;
                // std::cout<<"          you can put the process into model during trace"<<std::endl;
                // std::cout<<"          which can reduce post process time, but forward time increases 1ms"<<std::endl;
                // std::cout<<std::endl;
                const std::vector<std::vector<std::vector<int>>> anchors{
                    {{10, 13}, {16, 30}, {33, 23}},
                    {{30, 61}, {62, 45}, {59, 119}},
                    {{116, 90}, {156, 198}, {373, 326}}};
                const int anchor_num = anchors[0].size();
                assert(pSophgoContext->output_num == (int)anchors.size());
                assert(box_num>0);
                if((int)decoded_data.size() != box_num*nout){
                    decoded_data.resize(box_num*nout);
                }
                float *dst = decoded_data.data();
                for(int tidx = 0; tidx < pSophgoContext->output_num; ++tidx) {
                    auto output_tensor = outputTensors[tidx];
                    int feat_c = output_tensor->get_shape()->dims[1];
                    int feat_h = output_tensor->get_shape()->dims[2];
                    int feat_w = output_tensor->get_shape()->dims[3];
                    int area = feat_h * feat_w;
                    assert(feat_c == anchor_num);
                    int feature_size = feat_h*feat_w*nout;
                    float *tensor_data = (float*)output_tensor->get_cpu_data() + batch_idx*feat_c*area*nout;
                    for (int anchor_idx = 0; anchor_idx < anchor_num; anchor_idx++)
                    {
                    float *ptr = tensor_data + anchor_idx*feature_size;
                    for (int i = 0; i < area; i++) {
                        dst[0] = (sigmoid(ptr[0]) * 2 - 0.5 + i % feat_w) / feat_w * pSophgoContext->m_net_w;
                        dst[1] = (sigmoid(ptr[1]) * 2 - 0.5 + i / feat_w) / feat_h * pSophgoContext->m_net_h;
                        dst[2] = pow((sigmoid(ptr[2]) * 2), 2) * anchors[tidx][anchor_idx][0];
                        dst[3] = pow((sigmoid(ptr[3]) * 2), 2) * anchors[tidx][anchor_idx][1];
                        dst[4] = sigmoid(ptr[4]);
                        float score = dst[4];
                        if (score > pSophgoContext->m_thresh[0]) {
                        for(int d=5; d<nout; d++){
                            dst[d] = sigmoid(ptr[d]);
                        }
                        }
                        dst += nout;
                        ptr += nout;
                    }
                    }
                }
                output_data = decoded_data.data();
                } else {
                assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[1]);
                box_num = out_tensor->get_shape()->dims[1];
                output_data = (float*)out_tensor->get_cpu_data() + batch_idx*box_num*nout;
                }

                for (int i = 0; i < box_num; i++) {
                float* ptr = output_data+i*nout;
                float score = ptr[4];
                int class_id = argmax(&ptr[5], pSophgoContext->m_class_num);
                float confidence = ptr[class_id + 5];
                if (confidence * score > pSophgoContext->m_thresh[0])
                {
                    float centerX = (ptr[0]+1 - tx1)/ratio - 1;
                    float centerY = (ptr[1]+1 - ty1)/ratio - 1;
                    float width = (ptr[2]+0.5) / ratio;
                    float height = (ptr[3]+0.5) / ratio;

                    YoloV5Box box;
                    box.x = int(centerX - width / 2);
                    if (box.x < 0) box.x = 0;
                    box.y = int(centerY - height / 2);
                    if (box.y < 0) box.y = 0;
                    box.width = width;
                    box.height = height;
                    box.class_id = class_id;
                    box.score = confidence * score;
                    yolobox_vec.push_back(box);
                }
                }
                // printf("\n --> valid boxes number = %d\n", (int)yolobox_vec.size());
            #if USE_MULTICLASS_NMS
                std::vector<YoloV5BoxVec> class_vec(m_class_num);
                for (auto& box : yolobox_vec){
                class_vec[box.class_id].push_back(box);
                }
                for (auto& cls_box : class_vec){
                NMS(cls_box, m_nmsThreshold);
                }
                yolobox_vec.clear();
                for (auto& cls_box : class_vec){
                yolobox_vec.insert(yolobox_vec.end(), cls_box.begin(), cls_box.end());
                }
            #else
                NMS(yolobox_vec, pSophgoContext->m_thresh[1]);
            #endif

            for (auto bbox : yolobox_vec) {
              std::shared_ptr<common::ObjectMetadata> spObjData = std::make_shared<common::ObjectMetadata>();
              spObjData->mDetectedObjectMetadata = std::make_shared<common::DetectedObjectMetadata>();
              spObjData->mDetectedObjectMetadata->mBox.mX = bbox.x;
              spObjData->mDetectedObjectMetadata->mBox.mY = bbox.y;
              spObjData->mDetectedObjectMetadata->mBox.mWidth = bbox.width;
              spObjData->mDetectedObjectMetadata->mBox.mHeight = bbox.height;
              spObjData->mDetectedObjectMetadata->mScores.push_back(bbox.score);
              spObjData->mDetectedObjectMetadata->mClassify = bbox.class_id;
              objectMetadatas[batch_idx]->mSubObjectMetadatas.push_back(spObjData);
            }
            
#if 0            
                  int width = objectMetadatas[batch_idx]->mFrame->mWidth;
                  int height = objectMetadatas[batch_idx]->mFrame->mHeight;
                  std::shared_ptr<void> data = objectMetadatas[batch_idx]->mFrame->mData;
                  // 转格式
                  sophon_stream::common::FormatType format_type_stream = objectMetadatas[batch_idx]->mFrame->mFormatType;
                  sophon_stream::common::DataType data_type_stream = objectMetadatas[batch_idx]->mFrame->mDataType;
                  bm_image_format_ext format_type_bmcv = common::format_stream2bmcv(format_type_stream);
                  bm_image_data_format_ext data_type_bmcv = common::data_stream2bmcv(data_type_stream);
                  // 转成bm_image
                  bm_image image;
                  bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, format_type_bmcv, 
                  data_type_bmcv, &image);
                  bm_image_attach(image, objectMetadatas[batch_idx]->mFrame->mData.get());


                  bm_image imageStorage;
                  bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, FORMAT_YUV420P, image.data_type, &imageStorage);
                  bmcv_image_storage_convert(pSophgoContext->m_bmContext->handle(), 1, &image, &imageStorage);
                  bm_image_destroy(image);
                
                for (auto bbox : yolobox_vec) {
      #if DEBUG
                  cout << "  class id=" << bbox.class_id << ", score = " << bbox.score << " (x=" << bbox.x << ",y=" << bbox.y << ",w=" << bbox.width << ",h=" << bbox.height << ")" << endl;
      #endif
                  // draw image
                  draw_bmcv(pSophgoContext->m_bmContext->handle(), bbox.class_id, bbox.score, bbox.x, bbox.y, bbox.width, bbox.height, imageStorage);

                }

                // save image
                void* jpeg_data = NULL;
                size_t out_size = 0;
                int ret = bmcv_image_jpeg_enc(pSophgoContext->m_bmContext->handle(), 1, &imageStorage, &jpeg_data, &out_size);
                if (ret == BM_SUCCESS) {
                  std::string img_file = "a.jpg";
                  FILE *fp = fopen(img_file.c_str(), "wb");
                  fwrite(jpeg_data, out_size, 1, fp);
                  fclose(fp);
                }
                free(jpeg_data);
                bm_image_destroy(imageStorage);
#endif              

                //detected_boxes.push_back(yolobox_vec);
            }
        }


} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream