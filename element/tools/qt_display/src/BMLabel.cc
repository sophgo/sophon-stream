// ===----------------------------------------------------------------------===
//
//  Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
//  SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
//  third-party components.
//
// ===----------------------------------------------------------------------===

#include "BMLabel.h"

namespace sophon_stream {
namespace element {
namespace qt_display {

BMLabel::BMLabel(QWidget* parent, int width, int height) :QLabel(parent){
  setFixedSize(width, height);
  connect(this, &BMLabel::show_signals, this, &BMLabel::show_pixmap);

}

BMLabel::~BMLabel(){}

void BMLabel::show_img(std::shared_ptr<bm_image> bmimg_ptr) {
  int label_width = this->width();
  int label_height = this->height();

  cv::Mat mat_bgr;
  cv::bmcv::toMAT(bmimg_ptr.get(), mat_bgr, true);

  cv::Mat mat_resized;
  cv::resize(mat_bgr, mat_resized, cv::Size(label_width, label_height));

  // 公版qt版本>5.14，qimage才能支持bgr显示，但rgb都支持
  cv::Mat o_mat;
  cv::cvtColor(mat_resized, o_mat, cv::COLOR_BGR2RGB);

  QImage _image = QImage((uchar*)o_mat.data, label_width, label_height, o_mat.step,
    QImage::Format_RGB888).copy();
  image_pixmap = QPixmap::fromImage(_image);
  emit BMLabel::show_signals();
}

void BMLabel::show_pixmap() {
    this->setPixmap(image_pixmap);
    this->update();
}

}  // namespace qt_display
}  // namespace element
}  // namespace sophon_stream