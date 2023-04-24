#include "ElementManager.h"

#include <nlohmann/json.hpp>

#include "common/Logger.h"

#include "PostModuleElement.h"
#include "PreModuleElement.h"
#include "ElementFactory.h"

namespace sophon_stream {
namespace framework {

/** 
 * Constructor of class ElementManager.
 */
ElementManager::ElementManager()
    : mId(-1),mThreadStatus(ThreadStatus::STOP) {
}

/**
 * Destructor of class ElementManager.
 */
ElementManager::~ElementManager() {
    // uninit();
}

constexpr const char* ElementManager::JSON_GRAPH_ID_FIELD;

static constexpr const char* JSON_WORKERS_FIELD = "elements";
static constexpr const char* JSON_MODULES_FIELD = "modules";
static constexpr const char* JSON_CONNECTIONS_FIELD = "connections";
 
/**
 * Init ElementManager with configure in json format.
 * @param[in] json : Configure in json format. 
 * @return If parse configure fail, it will return error, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::init(const std::string& json) {
    IVS_INFO("Init start, json: {0}", json);

    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto graphIdIt = configure.find(JSON_GRAPH_ID_FIELD);
        if (configure.end() == graphIdIt
                || !graphIdIt->is_number_integer()) {
            IVS_ERROR("Can not find {0} with integer type in graph json configure, json: {1}",
                      JSON_GRAPH_ID_FIELD,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        mId = graphIdIt->get<int>();

        auto elementsIt = configure.find(JSON_WORKERS_FIELD);
        if (configure.end() != elementsIt) {
            errorCode = initElements(elementsIt->dump());
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
        }

        auto modulesIt = configure.find(JSON_MODULES_FIELD);
        if (configure.end() != modulesIt) {
            errorCode = initModules(modulesIt->dump());
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
        }

        auto connectionsIt = configure.find(JSON_CONNECTIONS_FIELD);
        if (configure.end() != connectionsIt) {
            errorCode = initConnections(connectionsIt->dump());
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
        }

    } while (false);

    if (common::ErrorCode::SUCCESS != errorCode) {
        uninit();
    }

    IVS_INFO("Init finish, json: {0}", json);
    return errorCode;
}

/**
 * Uninit ElementManager, will stop WorkManager.
 */
void ElementManager::uninit() {
    int id = mId;
    IVS_INFO("Uninit start, graph id: {0:d}", id);

    stop();

    mElementMap.clear();
    mId = -1;

    IVS_INFO("Uninit finish, graph id: {0:d}", id);
}

/** 
 * Start all threads of Elements in this ElementManager.
 * @return If thread status is not ThreadStatus::STOP, 
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::start() {
    IVS_INFO("Start graph thread start, graph id: {0:d}", mId);

    if (ThreadStatus::STOP != mThreadStatus) {
        IVS_ERROR("Can not start, current thread status is not stop, graph id: {0:d}",
                  mId);
        return common::ErrorCode::THREAD_STATUS_ERROR;
    }

    for (auto pair : mElementMap) {
        auto element = pair.second;
        if (!element) {
            continue;
        }

        element->start();
    }

    mThreadStatus = ThreadStatus::RUN;

    IVS_INFO("Start graph thread finish, graph id: {0:d}", mId);
    return common::ErrorCode::SUCCESS;
}

/** 
 * Stop all threads of Elements in this ElementManager.
 * @return If thread status is ThreadStatus::STOP, 
 * it will return common::ErrorCode::THREAD_STATUS_ERROR, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::stop() {
    IVS_INFO("Stop graph thread start, graph id: {0:d}", mId);

    if (ThreadStatus::STOP == mThreadStatus) {
        IVS_ERROR("Can not stop, current thread status is stop");
        return common::ErrorCode::THREAD_STATUS_ERROR;
    }

    for (auto pair : mElementMap) {
        auto element = pair.second;
        if (!element) {
            continue;
        }

        element->stop();
    }

    mThreadStatus = ThreadStatus::STOP;

    IVS_INFO("Stop graph thread finish, graph id: {0:d}", mId);
    return common::ErrorCode::SUCCESS;
}

/** 
 * Pause all threads of Elements in this ElementManager.
 * @return If thread status is not ThreadStatus::RUN, 
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::pause() {
    IVS_INFO("Pause graph thread start, graph id: {0:d}", mId);

    if (ThreadStatus::RUN != mThreadStatus) {
        IVS_ERROR("Can not pause, current thread status is not run, graph id: {0:d}",
                  mId);
        return common::ErrorCode::THREAD_STATUS_ERROR;
    }

    for (auto pair : mElementMap) {
        auto element = pair.second;
        if (!element) {
            continue;
        }

        element->pause();
    }

    mThreadStatus = ThreadStatus::PAUSE;

    IVS_INFO("Pause graph thread finish, graph id: {0:d}", mId);
    return common::ErrorCode::SUCCESS;
}

/** 
 * Resume all threads of Elements in this ElementManager.
 * @return If thread status is not ThreadStatus::PAUSE, 
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::resume() {
    IVS_INFO("Resume graph thread start, graph id: {0:d}", mId);

    if (ThreadStatus::PAUSE != mThreadStatus) {
        IVS_ERROR("Can not resume, current thread status is not pause, graph id: {0:d}",
                  mId);
        return common::ErrorCode::THREAD_STATUS_ERROR;
    }

    for (auto pair : mElementMap) {
        auto element = pair.second;
        if (!element) {
            continue;
        }

        element->resume();
    }

    mThreadStatus = ThreadStatus::RUN;

    IVS_INFO("Resume graph thread finish, graph id: {0:d}", mId);
    return common::ErrorCode::SUCCESS;
}

/** 
 * Constructor of struct Module.
 */
ElementManager::Module::Module()
    : mId(-1),
      mPreModuleElementId(-1),
      mPostModuleElementId(-1) {
}

/** 
 * Get first element id of Module.
 * @return Return first element id of Module.
 */
int ElementManager::Module::getFirstElementId() const {
    if (!mPreElementIds.empty()) {
        return mPreElementIds.front();
    }

    IVS_ERROR("Module has no pre element, module id: {0:d}", mId);
    return -1;
}

/** 
 * Get last element id of Module.
 * @return Return last element id of Module.
 */
int ElementManager::Module::getLastElementId() const {
    if (!mPostElementIds.empty()) {
        return mPostElementIds.back();
    }

    if (!mSubModuleIds.empty()) {
        return mPostModuleElementId;
    }

    return mPreElementIds.back();
}

static constexpr const char* JSON_WORKER_NAME_FIELD = "name";

/**
 * Init Elements with configure in json format.  
 * @param[in] json : Configure in json format.
 * @return If parse configure fail, it will return error, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::initElements(const std::string& json) {
    IVS_INFO("Init elements start, graph id: {0:d}, json: {1}",
             mId,
             json);

    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto elementsConfigure = nlohmann::json::parse(json, nullptr, false);
        if (!elementsConfigure.is_array()) {
            IVS_ERROR("Parse json fail or json is not array, graph id: {0:d}, json: {1}",
                      mId,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        for (auto elementConfigure : elementsConfigure) {
            if (!elementConfigure.is_object()) {
                IVS_ERROR("Element json configure is not object, graph id: {0:d}, json: {1}",
                          mId,
                          elementConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            auto nameIt = elementConfigure.find(JSON_WORKER_NAME_FIELD);
            if (elementConfigure.end() == nameIt
                    || !nameIt->is_string()) {
                IVS_ERROR("Can not find {0} with string type in element json configure, graph id: {1:d}, json: {2}",
                          JSON_WORKER_NAME_FIELD,
                          mId,
                          elementConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            auto& elementFactory = framework::SingletonElementFactory::getInstance();
            auto element = elementFactory.make(nameIt->get<std::string>());
            if (!element) {
                IVS_ERROR("Make element fail, graph id: {0:d}, name: {1}",
                          mId,
                          nameIt->get<std::string>());
                errorCode = common::ErrorCode::NO_SUCH_WORKER;
                break;
            }

            errorCode = element->init(elementConfigure.dump());
            if (common::ErrorCode::SUCCESS != errorCode) {
                IVS_ERROR("Init element fail, graph id: {0:d}, name: {1}",
                          mId,
                          nameIt->get<std::string>());
                break;
            }

            if (mElementMap.end() != mElementMap.find(element->getId())) {
                IVS_ERROR("Repeated element id, graph id: {0:d}, element id: {1:d}",
                          mId,
                          element->getId());
                errorCode = common::ErrorCode::REPEATED_WORKER_ID;
                break;
            }

            mElementMap[element->getId()] = element;
        }
        if (common::ErrorCode::SUCCESS != errorCode) {
            break;
        }

    } while (false);

    IVS_INFO("Init elements finish, graph id: {0:d}, json: {1}",
             mId,
             json);
    return errorCode;
}

static constexpr const char* JSON_MODULE_ID_FIELD = "id";
static constexpr const char* JSON_MODULE_PRE_WORKER_IDS_FIELD = "pre_element_ids";
static constexpr const char* JSON_MODULE_MODULE_IDS_FIELD = "module_ids";
static constexpr const char* JSON_MODULE_POST_WORKER_IDS_FIELD = "post_element_ids";

/**
 * Init Modules with configure in json format.  
 * @param[in] json : Configure in json format.
 * @return If parse configure fail, it will return error, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::initModules(const std::string& json) {
    IVS_INFO("Init modules start, graph id: {0:d}, json: {1}",
             mId,
             json);

    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto modulesConfigure = nlohmann::json::parse(json, nullptr, false);
        if (!modulesConfigure.is_array()) {
            IVS_ERROR("Parse json fail or json is not array, graph id: {0:d}, json: {1}",
                      mId,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        /* begin init module map */
        for (auto moduleConfigure : modulesConfigure) {
            if (!moduleConfigure.is_object()) {
                IVS_ERROR("Module json configure is not object, graph id: {0:d}, json: {1}",
                          mId,
                          moduleConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            auto moduleIdIt = moduleConfigure.find(JSON_MODULE_ID_FIELD);
            if (moduleConfigure.end() == moduleIdIt
                    || !moduleIdIt->is_number_integer()) {
                IVS_ERROR("Can not find {0} with integer type in module json configure, graph id: {1:d}, json: {2}",
                          JSON_MODULE_ID_FIELD,
                          mId,
                          moduleConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            auto module = std::make_shared<Module>();
            module->mId = *moduleIdIt;

            auto preElementIdsIt = moduleConfigure.find(JSON_MODULE_PRE_WORKER_IDS_FIELD);
            if (moduleConfigure.end() == preElementIdsIt
                    || !preElementIdsIt->is_array()
                    || preElementIdsIt->empty()) {
                IVS_ERROR("Can not find a no empty {0} with array type in module json configure, graph id: {1:d}, json: {2}",
                          JSON_MODULE_PRE_WORKER_IDS_FIELD,
                          mId,
                          moduleConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            module->mPreElementIds.reserve(preElementIdsIt->size());
            for (auto preElementIdConfigure : *preElementIdsIt) {
                if (!preElementIdConfigure.is_number_integer()) {
                    IVS_ERROR("Pre element id json configure is not integer, graph id: {0:d}, json: {1}",
                              mId,
                              preElementIdConfigure.dump());
                    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                    break;
                }

                module->mPreElementIds.push_back(preElementIdConfigure);
            }
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }

            auto postElementIdsIt = moduleConfigure.find(JSON_MODULE_POST_WORKER_IDS_FIELD);
            if (moduleConfigure.end() != postElementIdsIt
                    && postElementIdsIt->is_array()
                    && !postElementIdsIt->empty()) {

                module->mPostElementIds.reserve(postElementIdsIt->size());
                for (auto postElementIdConfigure : *postElementIdsIt) {
                    if (!postElementIdConfigure.is_number_integer()) {
                        IVS_ERROR("Post element id json configure is not integer, graph id: {0:d}, json: {0}",
                                  mId,
                                  postElementIdConfigure.dump());
                        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                        break;
                    }

                    module->mPostElementIds.push_back(postElementIdConfigure);
                }
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }
            }

            auto moduleIdsIt = moduleConfigure.find(JSON_MODULE_MODULE_IDS_FIELD);
            if (moduleConfigure.end() != moduleIdsIt
                    && moduleIdsIt->is_array()
                    && !moduleIdsIt->empty()) {
                auto& elementFactory = framework::SingletonElementFactory::getInstance();

                /* begin make pre process element */
                while (mElementMap.end() != mElementMap.find(framework::PreModuleElement::gCurrentElementId)
                        && mModuleMap.end() != mModuleMap.find(framework::PreModuleElement::gCurrentElementId)
                        && module->mId != framework::PreModuleElement::gCurrentElementId) {
                    ++framework::PreModuleElement::gCurrentElementId;
                }

                module->mPreModuleElementId = framework::PreModuleElement::gCurrentElementId++;

                auto preModuleElement = elementFactory.make(framework::PreModuleElement::NAME);
                if (!preModuleElement) {
                    IVS_ERROR("Make element fail, graph id: {0:d}, name: {1}",
                              mId,
                              framework::PreModuleElement::NAME);
                    errorCode = common::ErrorCode::NO_SUCH_WORKER;
                    break;
                }

                nlohmann::json preModuleElementConfigure;
                preModuleElementConfigure[framework::Element::JSON_ID_FIELD] = module->mPreModuleElementId;
                preModuleElementConfigure[framework::Element::JSON_CONFIGURE_FIELD]
                [framework::PreModuleElement::JSON_MODULE_COUNT_FIELD] = moduleIdsIt->size();

                errorCode = preModuleElement->init(preModuleElementConfigure.dump());
                if (common::ErrorCode::SUCCESS != errorCode) {
                    IVS_ERROR("Init element fail, graph id: {0:d}, name: {1}",
                              mId,
                              framework::PreModuleElement::NAME);
                    break;
                }

                mElementMap[preModuleElement->getId()] = preModuleElement;
                /* end make pre process element */

                /* begin make post process element */
                while (mElementMap.end() != mElementMap.find(framework::PostModuleElement::gCurrentElementId)
                        && mModuleMap.end() != mModuleMap.find(framework::PostModuleElement::gCurrentElementId)
                        && module->mId != framework::PostModuleElement::gCurrentElementId) {
                    ++framework::PostModuleElement::gCurrentElementId;
                }

                module->mPostModuleElementId = framework::PostModuleElement::gCurrentElementId++;

                auto postModuleElement = elementFactory.make(framework::PostModuleElement::NAME);
                if (!postModuleElement) {
                    IVS_ERROR("Make element fail, graph id: {0:d}, name: {1}",
                              mId,
                              framework::PostModuleElement::NAME);
                    errorCode = common::ErrorCode::NO_SUCH_WORKER;
                    break;
                }

                nlohmann::json postModuleElementConfigure;
                postModuleElementConfigure[framework::Element::JSON_ID_FIELD] = module->mPostModuleElementId;
                postModuleElementConfigure[framework::Element::JSON_CONFIGURE_FIELD]
                [framework::PostModuleElement::JSON_MODULE_COUNT_FIELD] = moduleIdsIt->size();

                errorCode = postModuleElement->init(postModuleElementConfigure.dump());
                if (common::ErrorCode::SUCCESS != errorCode) {
                    IVS_ERROR("Init element fail, graph id: {0:d}, name: {1}",
                              mId,
                              framework::PostModuleElement::NAME);
                    break;
                }

                mElementMap[postModuleElement->getId()] = postModuleElement;
                /* end make post process element */

                module->mSubModuleIds.reserve(moduleIdsIt->size());
                for (auto moduleIdConfigure : *moduleIdsIt) {
                    if (!moduleIdConfigure.is_number_integer()) {
                        IVS_ERROR("Module id json configure is not integer, graph id: {0:d}, json: {1}",
                                  mId,
                                  moduleIdConfigure.dump());
                        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                        break;
                    }

                    module->mSubModuleIds.push_back(moduleIdConfigure);
                }
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }
            }

            mModuleMap[module->mId] = module;
        }
        if (common::ErrorCode::SUCCESS != errorCode) {
            break;
        }
        /* end init module map */

        /* begin connect */
        for (auto pair : mModuleMap) {
            auto module = pair.second;
            int lastElementId = -1;

            /* begin connect pre elements */
            bool firstElement = true;
            for (auto& preElementId : module->mPreElementIds) {
                if (firstElement) {
                    firstElement = false;
                } else {
                    errorCode = connect(lastElementId,
                                        0,
                                        preElementId,
                                        0);
                    if (common::ErrorCode::SUCCESS != errorCode) {
                        break;
                    }
                }

                lastElementId = preElementId;
            }
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
            /* end connect pre elements */

            /* begin connect sub modules */
            if (!module->mSubModuleIds.empty()) {
                errorCode = connect(lastElementId,
                                    0,
                                    module->mPreModuleElementId,
                                    0);
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }

                errorCode = connect(module->mPreModuleElementId,
                                    0,
                                    module->mPostModuleElementId,
                                    0);
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }

                lastElementId = module->mPostModuleElementId;

                for (int i = 0; i < module->mSubModuleIds.size(); ++i) {
                    int subModuleId = module->mSubModuleIds[i];
                    auto subModuleIt = mModuleMap.find(subModuleId);
                    if (mModuleMap.end() == subModuleIt) {
                        errorCode = common::ErrorCode::UNKNOWN;
                        break;
                    }

                    auto subModule = subModuleIt->second;
                    errorCode = connect(module->mPreModuleElementId,
                                        i + 1,
                                        subModule->getFirstElementId(),
                                        0);
                    if (common::ErrorCode::SUCCESS != errorCode) {
                        break;
                    }

                    errorCode = connect(subModule->getLastElementId(),
                                        0,
                                        module->mPostModuleElementId,
                                        1);
                    if (common::ErrorCode::SUCCESS != errorCode) {
                        break;
                    }
                }
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }
            }
            /* end connect sub modules */

            /* begin connect post elements */
            for (auto& postElementId : module->mPostElementIds) {
                errorCode = connect(lastElementId,
                                    0,
                                    postElementId,
                                    0);
                if (common::ErrorCode::SUCCESS != errorCode) {
                    break;
                }

                lastElementId = postElementId;
            }
            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
            /* end connect post elements */
        }
        if (common::ErrorCode::SUCCESS != errorCode) {
            break;
        }
        /* end connect */

    } while (false);

    IVS_INFO("Init modules finish, graph id: {0:d}, json: {1}",
             mId,
             json);
    return errorCode;
}

static constexpr const char* JSON_CONNECTION_SRC_ID_FIELD = "src_id";
static constexpr const char* JSON_CONNECTION_SRC_PORT_FIELD = "src_port";
static constexpr const char* JSON_CONNECTION_DST_ID_FIELD = "dst_id";
static constexpr const char* JSON_CONNECTION_DST_PORT_FIELD = "dst_port";

/**
 * Init Connections with configure in json format.  
 * @param[in] json : Configure in json format.
 * @return If parse configure fail, it will return error, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::initConnections(const std::string& json) {
    IVS_INFO("Init connections start, graph id: {0:d}, json: {1}",
             mId,
             json);

    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto connectionsConfigure = nlohmann::json::parse(json, nullptr, false);
        if (!connectionsConfigure.is_array()) {
            IVS_ERROR("Parse json fail or json is not array, graph id: {0:d}, json: {1}",
                      mId,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        for (auto connectionConfigure : connectionsConfigure) {
            if (!connectionConfigure.is_object()) {
                IVS_ERROR("Connection json configure is not object, graph id: {0:d}, json: {1}",
                          mId,
                          connectionConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            auto srcElementIdIt = connectionConfigure.find(JSON_CONNECTION_SRC_ID_FIELD);
            if (connectionConfigure.end() == srcElementIdIt
                    || !srcElementIdIt->is_number_integer()) {
                IVS_ERROR("Can not find {0} with integer type in connection json configure, graph id: {1:d}, json: {2}",
                          JSON_CONNECTION_SRC_ID_FIELD,
                          mId,
                          connectionConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            int srcElementPort = 0;
            auto srcElementPortIt = connectionConfigure.find(JSON_CONNECTION_SRC_PORT_FIELD);
            if (connectionConfigure.end() != srcElementPortIt
                    && srcElementPortIt->is_number_integer()) {
                srcElementPort = srcElementPortIt->get<int>();
            }

            auto dstElementIdIt = connectionConfigure.find(JSON_CONNECTION_DST_ID_FIELD);
            if (connectionConfigure.end() == dstElementIdIt
                    || !dstElementIdIt->is_number_integer()) {
                IVS_ERROR("Can not find {0} with integer type in connection json configure, graph id: {1:d}, json: {2}",
                          JSON_CONNECTION_DST_ID_FIELD,
                          mId,
                          connectionConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            int dstElementPort = 0;
            auto dstElementPortIt = connectionConfigure.find(JSON_CONNECTION_DST_PORT_FIELD);
            if (connectionConfigure.end() != dstElementPortIt
                    && dstElementPortIt->is_number_integer()) {
                dstElementPort = dstElementPortIt->get<int>();
            }

            errorCode = connect(srcElementIdIt->get<int>(),
                                srcElementPort,
                                dstElementIdIt->get<int>(),
                                dstElementPort);

            if (common::ErrorCode::SUCCESS != errorCode) {
                break;
            }
        }
        if (common::ErrorCode::SUCCESS != errorCode) {
            break;
        }

    } while (false);

    IVS_INFO("Init connections finish, graph id: {0:d}, json: {1}",
             mId,
             json);
    return errorCode;
}

/**
 * Make a connectiton between a Element/Module and another Element/Module.  
 * @param[in] srcId : Id of source Element/Module, 
 * if it is Module's id, will use last Element of the Module instead.  
 * @param[in] srcPort : Output port of source Element or last Element of source Module.
 * @param[in] dstId : Id of destination Element/Module, 
 * if it is Module's id, will use first Element of the Module instead.  
 * @param[in] dstPort : Input port of destination Element or first Element of destination Module.
 * @return If can not find Element, Module or Element of Module, it will return error, 
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode ElementManager::connect(int srcId,
                       int srcPort,
                       int dstId,
                       int dstPort) {
    auto srcElementIt = mElementMap.find(srcId);
    if (mElementMap.end() == srcElementIt) {
        auto srcModuleIt = mModuleMap.find(srcId);
        if (mModuleMap.end() == srcModuleIt) {
            IVS_ERROR("Can not find module or element, graph id: {0:d}, module or element id: {1:d}",
                      mId,
                      srcId);
            return common::ErrorCode::NO_SUCH_WORKER_ID;
        }

        srcId = srcModuleIt->second->getLastElementId();
        srcElementIt = mElementMap.find(srcId);
        if (mElementMap.end() == srcElementIt) {
            IVS_ERROR("Can not find element, graphd id: {0:d}, element id: {1:d}",
                      mId,
                      srcId);
            return common::ErrorCode::NO_SUCH_WORKER_ID;
        }
    }

    auto srcElement = srcElementIt->second;
    if (!srcElement) {
        IVS_ERROR("Element is null, graph id: {0:d}, element id: {1:d}",
                  mId,
                  srcId);
        return common::ErrorCode::UNKNOWN;
    }

    auto dstElementIt = mElementMap.find(dstId);
    if (mElementMap.end() == dstElementIt) {
        auto dstModuleIt = mModuleMap.find(dstId);
        if (mModuleMap.end() == dstModuleIt) {
            IVS_ERROR("Can not find module or element, graph id: {0:d}, module or element id: {1:d}",
                      mId,
                      dstId);
            return common::ErrorCode::NO_SUCH_WORKER_ID;
        }

        dstId = dstModuleIt->second->getFirstElementId();
        dstElementIt = mElementMap.find(dstId);
        if (mElementMap.end() == dstElementIt) {
            IVS_ERROR("Can not find element, graph id: {0:d}, element id: {1:d}",
                      mId,
                      dstId);
            return common::ErrorCode::NO_SUCH_WORKER_ID;
        }
    }

    auto dstElement = dstElementIt->second;
    if (!dstElement) {
        IVS_ERROR("Can not find element, graph id: {0:d}, element id: {0:d}",
                  mId,
                  dstId);
        return common::ErrorCode::UNKNOWN;
    }

    framework::Element::connect(*srcElement,
                               srcPort,
                               *dstElement,
                               dstPort);

    IVS_INFO("{0}~~~~~~~~~~~~~~~~~~{1}",srcId,dstId);

    return common::ErrorCode::SUCCESS;
}

} // namespace framework
} // namespace sophon_stream

