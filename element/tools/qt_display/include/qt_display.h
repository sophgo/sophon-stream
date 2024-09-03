#ifndef SOPHON_STREAM_ELEMENT_QT_DISPLAY_H_
#define SOPHON_STREAM_ELEMENT_QT_DISPLAY_H_

#include <QLoggingCategory>
#include <QSharedPointer>
#include <atomic>
#include <unordered_set>

#include "BMLabel.h"
#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace qt_display {

class QtDisplay : public ::sophon_stream::framework::Element {
 public:
  QtDisplay();
  ~QtDisplay() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_SCREEN_WIDTH = "width";
  static constexpr const char* CONFIG_INTERNAL_SCREEN_HEIGHT = "height";

  static constexpr const char* CONFIG_INTERNAL_ROWS = "rows";
  static constexpr const char* CONFIG_INTERNAL_COLS = "cols";

  int qt_func();

 private:
  QApplication* qapp;
  QWidget* qwidget_ptr;
  QGridLayout* layout;

  int screen_width;
  int screen_height;
  int rows;
  int cols;
  std::vector<std::shared_ptr<BMLabel>> label_vec;

  std::thread qt_thread;

  // 停止qt的条件
  std::mutex channel_mutex;
  std::unordered_set<int> channel_ids;
  std::atomic<int> stopped_num;

  // std::vector<::sophon_stream::common::FpsProfiler*> mFpsProfilers;
  std::unordered_map<unsigned int, common::FpsProfiler*> mFpsProfilers;
  int thread_num;
};

}  // namespace qt_display
}  // namespace element
}  // namespace sophon_stream

#endif