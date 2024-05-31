// ===----------------------------------------------------------------------===
//
//  Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
//  SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
//  third-party components.
//
// ===----------------------------------------------------------------------===

#include "qt_display.h"
#include <QScreen>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace qt_display {

QtDisplay::QtDisplay() {}
QtDisplay::~QtDisplay() {
  qt_thread.join();
  for (int i = 0; i < thread_num; i++){
    delete mFpsProfilers[i];
  }
}

int QtDisplay::qt_func() {
  int q_argc = 1;
  char* q_argv = {"main"};

  qapp = new QApplication(q_argc, &q_argv);
  qwidget_ptr = new QWidget;

  qwidget_ptr->setGeometry(0, 0, screen_width, screen_height);
  layout = new QGridLayout(qwidget_ptr);
  qwidget_ptr->setLayout(layout);
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      std::shared_ptr<BMLabel> label_ptr =
          std::make_shared<BMLabel>(qwidget_ptr);
      layout->addWidget(label_ptr.get(), row, col);
      label_vec.push_back(label_ptr);
    }
  }

  qwidget_ptr->show();

  return qapp->exec();
}

common::ErrorCode QtDisplay::initInternal(const std::string& json) {
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object())
    return common::ErrorCode::PARSE_CONFIGURE_FAIL;

  screen_width = configure.find(CONFIG_INTERNAL_SCREEN_WIDTH)->get<int>();
  screen_height = configure.find(CONFIG_INTERNAL_SCREEN_HEIGHT)->get<int>();

  rows = configure.find(CONFIG_INTERNAL_ROWS)->get<int>();
  cols = configure.find(CONFIG_INTERNAL_COLS)->get<int>();
  stopped_num = 0;

  qt_thread = std::thread(&QtDisplay::qt_func, this);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  thread_num = getThreadNumber();

  for (int i = 0; i < thread_num; i++) {
    ::sophon_stream::common::FpsProfiler* mFpsProfiler =
        new ::sophon_stream::common::FpsProfiler();
    mFpsProfilers.push_back(mFpsProfiler);
    mFpsProfilers[i]->config("qt_display_" + std::to_string(i), 100);
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode QtDisplay::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr)
    return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  int channel_id = objectMetadata->mFrame->mChannelIdInternal;
  if (channel_ids.find(channel_id) == channel_ids.end()) {
    std::unique_lock<std::mutex> lock(channel_mutex);
    channel_ids.insert(channel_id);
  }
  auto bmimg_ptr = objectMetadata->mFrame->mSpDataOsd;

  if (bmimg_ptr == nullptr)
    bmimg_ptr = objectMetadata->mFrame->mSpData;

  if (objectMetadata->mFrame->mEndOfStream)
    stopped_num++;
  else
    label_vec[channel_id]->show_img(bmimg_ptr);

  if (stopped_num == channel_ids.size())
    qapp->quit();

  mFpsProfilers[channel_id]->add(1);

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  common::ErrorCode errorCode =
      pushOutputData(outputPort, outDataPipeId,
                     std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("qt_display", QtDisplay)

}  // namespace qt_display
}  // namespace element
}  // namespace sophon_stream

int main() {}