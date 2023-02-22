/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Yang Jun <jun.yang@lynxi.com>, 20-3-6

**************************************************/

#include "HostContext.h"
#include <nlohmann/json.hpp>

#define JSON_ALGORITHM_NAME_FIELD "algorithm_name"
#define JSON_ALGORITHM_MODEL_PATH_FIELD "model_path"
#define JSON_ALGORITHM_MAX_BATCHSIZE_FIELD "max_batchsize"
#define JSON_ALGORITHM_INPUT_NODE_NAME_FIELD "input_node_name"
#define JSON_ALGORITHM_INPUT_SHAPE_FIELD "input_shape"
#define JSON_ALGORITHM_NUM_INPUTS_FIELD "num_inputs"
#define JSON_ALGORITHM_OUTPUT_NODE_NAME_FIELD "output_node_name"
#define JSON_ALGORITHM_OUTPUT_SHAPE_FIELD "output_shape"
#define JSON_ALGORITHM_NUM_OUTPUTS_FIELD "num_outputs"
#define JSON_ALGORITHM_THRETHOLD_FIELD "threthold"
#define JSON_ALGORITHM_LABEL_NAMES_FIELD "label_names"
#define JSON_ALGORITHM_NUM_CLASS_FIELD "num_class"

#define JSON_ALGORITHM_TRACK_IOU "track_Iou"
#define JSON_ALGORITHM_TRACK_MAX_AGE "track_MaxAge"
#define JSON_ALGORITHM_TRACK_MIN_HINS "track_MinHins"
#define JSON_ALGORITHM_TRACK_UPDATE_TIMES "track_UpdateTimes"

#define JSON_ALGORITHM_TRACK_TOPN "track_TopN"
#define JSON_ALGORITHM_TRACK_BASE_TIMES "track_BaseTimes"
#define JSON_ALGORITHM_QUALITY_THRES_AREA "quality_ta"
#define JSON_ALGORITHM_QUALITY_THRES_RATIOLOWBOUND "quality_trl"
#define JSON_ALGORITHM_QUALITY_THRES_RATIOUPBOUND "quality_tru"
#define JSON_ALGORITHM_QUALITY_WIDTH "quality_w"
#define JSON_ALGORITHM_QUALITY_HEIGHT "quality_h"
#define JSON_ALGORITHM_QUALITY_MARGIN "quality_mg"
#define JSON_ALGORITHM_QUALITY_MAX_QUSCORE "quality_maxSc"
#define JSON_ALGORITHM_QUALITY_lATERAL_SIDE "quality_ls"

namespace lynxi {
namespace ivs {
namespace algorithm {
namespace context {

/**
 * context初始化
 * @param[in] json: 初始化的json字符串
 * @return 错误码
 */
common::ErrorCode HostContext::init(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto maxBatchSizeCon = configure.find(JSON_ALGORITHM_MAX_BATCHSIZE_FIELD);
        if (configure.end() != maxBatchSizeCon
                && maxBatchSizeCon->is_number_integer()) {
            maxBatchSize = maxBatchSizeCon->get<int>();
        }

        auto numClassCon = configure.find(JSON_ALGORITHM_NUM_CLASS_FIELD);
        if (configure.end() != numClassCon
                && numClassCon->is_number_integer()) {
            numClass = numClassCon->get<int>();
        }

        auto modelPathIter = configure.find(JSON_ALGORITHM_MODEL_PATH_FIELD);
        if(modelPathIter!=configure.end()&&modelPathIter->is_array()){
            for(auto& temp : *modelPathIter) {
    //            if (configure.end() != temp
    //                    && temp.is_string()) {
                modelPath.push_back(temp.get<std::string>());
    //            }
            }

        }

        auto inputNodeCon = configure.find(JSON_ALGORITHM_INPUT_NODE_NAME_FIELD);
        if(inputNodeCon!=configure.end()&&inputNodeCon->is_array()){
            for(auto& temp : *inputNodeCon) {
    //            if (configure.end() != temp
    //                    && temp.is_string()) {
                inputNodeName.push_back(temp.get<std::string>());
    //            }
            }
        }

        auto inputShapeCon = configure.find(JSON_ALGORITHM_INPUT_SHAPE_FIELD);
        if(inputShapeCon!=configure.end()&&inputShapeCon->is_array()){
            for(auto& temp : *inputShapeCon) {
    //            if (configure.end() != temp
    //                    && temp.is_array()) {
                nodeDims node;
                for(int i=0;i<temp.size();++i){
                    if(i==0) node.c = temp[i].get<int>();
                    else if(i==1) node.h = temp[i].get<int>();
                    else if(i==2) node.w = temp[i].get<int>();
                }
                inputShape.push_back(node);
    //            }
            }
        }

        auto numInputsCon = configure.find(JSON_ALGORITHM_NUM_INPUTS_FIELD);
        if(numInputsCon!=configure.end()&&numInputsCon->is_array()){
            for(auto& temp : *numInputsCon) {
    //            if (configure.end() != temp
    //                    && temp.is_number_integer()) {
                numInputs.push_back(temp.get<int>());
    //            }
            }
        }

        auto outputNodeCon = configure.find(JSON_ALGORITHM_OUTPUT_NODE_NAME_FIELD);
        if(outputNodeCon!=configure.end()&&outputNodeCon->is_array()){
            for(auto& temp : *outputNodeCon) {
    //            if (configure.end() != temp
    //                    && temp.is_string()) {
                outputNodeName.push_back(temp.get<std::string>());
    //            }
            }
        }

        auto outputShapeCon = configure.find(JSON_ALGORITHM_OUTPUT_SHAPE_FIELD);
        if(outputShapeCon!=configure.end()&&outputShapeCon->is_array()){
            for(auto& temp : *outputShapeCon) {
    //            if (configure.end() != temp
    //                    && temp.is_array()) {
                nodeDims node;
                for(int i=0;i<temp.size();++i){
                    if(i==0) node.c = temp[i].get<int>();
                    else if(i==1) node.h = temp[i].get<int>();
                    else if(i==2) node.w = temp[i].get<int>();
                }
                outputShape.push_back(node);
    //            }
            }
        }

        auto numOutputsCon = configure.find(JSON_ALGORITHM_NUM_OUTPUTS_FIELD);
        if(numOutputsCon!=configure.end()&&numOutputsCon->is_array()){
            for(auto& temp : *numOutputsCon) {
    //            if (configure.end() != temp
    //                    && temp.is_number_integer()) {
                numOutputs.push_back(temp.get<int>());
    //            }
            }
        }

        auto thretholdCon = configure.find(JSON_ALGORITHM_THRETHOLD_FIELD);
        if(thretholdCon!=configure.end()&&thretholdCon->is_array()){
            for(auto& temp : *thretholdCon) {
    //            if (configure.end() != temp
    //                    && temp.is_number_float()) {
                threthold.push_back(temp.get<float>());
    //            }
            }
        }

        auto labelNamesCon = configure.find(JSON_ALGORITHM_LABEL_NAMES_FIELD);
        if(labelNamesCon!=configure.end()&&labelNamesCon->is_array()){
            for(auto& temp : *labelNamesCon) {
    //            if (configure.end() != temp
    //                    && temp.is_string()) {
                labelNames.push_back(temp.get<std::string>());
    //            }
            }
        }
        //TODO: parse other configure field
        //track config
        auto trackIouIter = configure.find(JSON_ALGORITHM_TRACK_IOU);
        if(trackIouIter!=configure.end()&&trackIouIter->is_number_float()){
            mTrackConfig.mIou = trackIouIter->get<float>();
        }

        auto maxAgeIter = configure.find(JSON_ALGORITHM_TRACK_MAX_AGE);
        if(maxAgeIter!=configure.end()&&maxAgeIter->is_number_integer()){
            mTrackConfig.mMaxAge = maxAgeIter->get<int>();
        }

        auto minHinsIter = configure.find(JSON_ALGORITHM_TRACK_MIN_HINS);
        if(minHinsIter!=configure.end()&&minHinsIter->is_number_integer()){
            mTrackConfig.mMinhins = minHinsIter->get<int>();
        }

        auto updateTimesIter = configure.find(JSON_ALGORITHM_TRACK_UPDATE_TIMES);
        if(updateTimesIter!=configure.end()&&updateTimesIter->is_number_integer()){
            mTrackConfig.mUpdateTimes = updateTimesIter->get<int>();
        }

        //quality config
        auto topnIter = configure.find(JSON_ALGORITHM_TRACK_TOPN);
        auto basetimeIter = configure.find(JSON_ALGORITHM_TRACK_BASE_TIMES);
        auto areaIter = configure.find(JSON_ALGORITHM_QUALITY_THRES_AREA);
        auto lowboundIter = configure.find(JSON_ALGORITHM_QUALITY_THRES_RATIOLOWBOUND);
        auto upboundIter = configure.find(JSON_ALGORITHM_QUALITY_THRES_RATIOUPBOUND);
        auto widthIter = configure.find(JSON_ALGORITHM_QUALITY_WIDTH);
        auto heightIter = configure.find(JSON_ALGORITHM_QUALITY_HEIGHT);
        auto marginIter = configure.find(JSON_ALGORITHM_QUALITY_MARGIN);
        auto maxScoreIter = configure.find(JSON_ALGORITHM_QUALITY_MAX_QUSCORE);
        auto lateralSideIter = configure.find(JSON_ALGORITHM_QUALITY_lATERAL_SIDE);
        if(topnIter!=configure.end()&&topnIter->is_number_integer()&&
                basetimeIter!=configure.end()&&basetimeIter->is_number_integer()&&
                areaIter!=configure.end()&&areaIter->is_number_float()&&
                lowboundIter!=configure.end()&&lowboundIter->is_number_float()&&
                upboundIter!=configure.end()&&upboundIter->is_number_float()&&
                widthIter!=configure.end()&&widthIter->is_number_float()&&
                heightIter!=configure.end()&&heightIter->is_number_float()&&
                marginIter!=configure.end()&&marginIter->is_number_float()&&
                maxScoreIter!=configure.end()&&maxScoreIter->is_number_float()&&
                lateralSideIter!=configure.end()&&lateralSideIter->is_number_float()){
            mQualityFilterConfig.mTopn = topnIter->get<int>();
            mQualityFilterConfig.mBaseTime = basetimeIter->get<int>();
            tracker_sort::QualityConfig qualityConfig(areaIter->get<float>(),lowboundIter->get<float>(), upboundIter->get<float>(), 
                    widthIter->get<float>(), heightIter->get<float>(), marginIter->get<float>(), maxScoreIter->get<float>(), 
                    lateralSideIter->get<float>());
            mQualityFilterConfig.mSpQualityControl = std::make_shared<tracker_sort::QualityControl>(qualityConfig);
        }
        

    } while (false);

    return errorCode;
}
} // namespace context
} // namespace algorithm
} // namespace ivs
} // namespace lynxi


