#include <nlohmann/json.hpp>

nlohmann::json makeAlgorithmConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   std::vector<std::string> modelPath, int maxBatchSize,
                                   std::vector<std::string> inputNodeName,
                                   std::vector<int> numInputs,
                                   std::vector<std::vector<int>> inputShape,
                                   std::vector<std::string> outputNodeName,
                                   std::vector<int> numOutputs,
                                   std::vector<std::vector<int>> outputShape,
                                   std::vector<float> threthold,
                                   int numClass, 
                                   std::vector<std::string> labelNames={}) {
    nlohmann::json algorithmConfigure;
    algorithmConfigure["shared_object"] = sharedObject;
    algorithmConfigure["name"] = name;
    algorithmConfigure["algorithm_name"] = algorithmName;
    algorithmConfigure["model_path"] = modelPath;
    algorithmConfigure["max_batchsize"] = maxBatchSize;
    algorithmConfigure["input_node_name"] = inputNodeName;
    algorithmConfigure["num_inputs"] = numInputs;
    algorithmConfigure["input_shape"] = inputShape;
    algorithmConfigure["output_node_name"] = outputNodeName;
    algorithmConfigure["num_outputs"] = numOutputs;
    algorithmConfigure["output_shape"] = outputShape;
    algorithmConfigure["threthold"] = threthold;
    algorithmConfigure["num_class"] = numClass;
    algorithmConfigure["label_names"] = labelNames;
    return algorithmConfigure;
}

nlohmann::json makeEncodeConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   int maxBatchSize
                                   ) {
    nlohmann::json algorithmConfigure;
    algorithmConfigure["shared_object"] = sharedObject;
    algorithmConfigure["name"] = name;
    algorithmConfigure["algorithm_name"] = algorithmName;
    algorithmConfigure["max_batchsize"] = maxBatchSize;
        return algorithmConfigure;
}

nlohmann::json makeWorkerConfig(int workerId,std::string workerName,
                                std::string side,int deviceId,
                                int threadNumber,int timeout,bool repeatTimeout,
                                int batch,
                                std::vector<nlohmann::json> algoConfig) {
    nlohmann::json workerConf;
    workerConf["id"] = workerId;
    workerConf["name"] = workerName;
    workerConf["side"] = side;
    workerConf["device_id"] = deviceId;
    workerConf["thread_number"] = threadNumber;
    workerConf["milliseconds_timeout"] = timeout;
    workerConf["repeated_timeout"] = repeatTimeout;

    nlohmann::json moduleConfigure;
    moduleConfigure["batch"] = batch;
    for(auto& ac:algoConfig) {
        moduleConfigure["models"].push_back(ac);
    }
    workerConf["configure"] = moduleConfigure;
    return workerConf;
}

nlohmann::json makeDecoderWorkerConfig(int workerId,std::string workerName,
                                std::string side,int deviceId,
                                int threadNumber,int timeout,bool repeatTimeout,
                                int batch,
                                 const std::string& soPath) {
    nlohmann::json workerConf;
    workerConf["id"] = workerId;
    workerConf["name"] = workerName;
    workerConf["side"] = side;
    workerConf["device_id"] = deviceId;
    workerConf["thread_number"] = threadNumber;
    workerConf["milliseconds_timeout"] = timeout;
    workerConf["repeated_timeout"] = repeatTimeout;

    nlohmann::json moduleConfigure;
    moduleConfigure["shared_object"] = soPath;
    workerConf["configure"] = moduleConfigure;
    return workerConf;
}
nlohmann::json makeModuleConfig(int id,std::vector<int> preWorkerIds,
                                std::vector<int> moduleIds,std::vector<int> postWorkerIds) {
    nlohmann::json config;
    config["id"] = id;
    for(auto& preWorkerId:preWorkerIds) {
        config["pre_worker_ids"].push_back(preWorkerId);
    }
    for(auto& moduleId:moduleIds) {
        config["module_ids"].push_back(moduleId);
    }
    for(auto& postWorker:postWorkerIds) {
        config["post_worker_ids"].push_back(postWorker);
    }
    return config;
}

nlohmann::json makeConnectConfig(int srcId,int srcPort,int dstId,int dstPort) {
    nlohmann::json connectConf;
    connectConf["src_id"] = srcId;
    connectConf["src_port"] = srcPort;
    connectConf["dst_id"] = dstId;
    connectConf["dst_port"] = dstPort;
    return connectConf;
}
