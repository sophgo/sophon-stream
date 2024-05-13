// ===----------------------------------------------------------------------===
//
//  Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
//  SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
//  third-party components.
//
// ===----------------------------------------------------------------------===

#ifndef SOPHON_STREAM_ELEMENT_QT_DISPLAY_BMLABEL_H_
#define SOPHON_STREAM_ELEMENT_QT_DISPLAY_BMLABEL_H_

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"

namespace sophon_stream {
namespace element {
namespace qt_display {

class BMLabel : public QLabel {
  Q_OBJECT
 public:
  explicit BMLabel(QWidget* parent = nullptr,
                   Qt::WindowFlags f = Qt::WindowFlags());
  ~BMLabel();

  void show_img(std::shared_ptr<bm_image> bmimg_ptr);

 public slots:

  void show_pixmap();

 signals:
  void show_signals();

 private:
  QPixmap image_pixmap;
};

}  // namespace qt_display
}  // namespace element
}  // namespace sophon_stream
#endif