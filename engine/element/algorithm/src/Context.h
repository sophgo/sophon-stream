#pragma once

#include <string>
#include <vector>
#include <memory>
#include "common/ErrorCode.h"
namespace sophon_stream {
namespace algorithm {

struct nodeDims {
    int c = 0;
    int h = 0;
    int w = 0;
};
/**
 * context基类
 */
struct Context {
    std::string algorithmName; //算法名字
    int deviceId; //设备ID
    int maxBatchSize; //最大batch
    int numBatch; //当前batch
    int numClass; //类别数目
    std::vector<std::string> modelPath; //模型路径
    std::vector<std::string> inputNodeName; //输入节点名字
    std::vector<nodeDims> inputShape; //输入shape
    std::vector<std::string> outputNodeName; //输出节点名字
    std::vector<nodeDims> outputShape; //输出shape
    std::vector<int> numInputs; //输入数量
    std::vector<int> numOutputs; //输出数量
    std::vector<float> threthold; //阈值
    std::vector<std::string> labelNames; //lablel名字
    std::shared_ptr<void> data = nullptr; //数据
    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    virtual common::ErrorCode init(const std::string&)=0;
};

} // namespace algorithm
} // namespace sophon_stream

