#ifndef CV_UNI_TEXT_HPP
#define CV_UNI_TEXT_HPP
#include <memory>
#include <opencv2/opencv.hpp>
#include <common/logger.h>
#include <common/common_defs.h>
#include <string>

namespace uni_text {
class Impl;

class UniText {
 public:
  /// Initialization
  /// \param font_face: the file path of your font
  /// \param font_size
  UniText(const std::string& font_face, int font_size);
  ~UniText();

  /// Detailed parameter of text rendering. Call this before drawing the text
  /// \param font_size
  /// \param interval_ratio: Ratio of character interval over character width
  /// \param whitespace_ratio: Ratio of whitespace width over character width
  /// \param alpha: Transparency. 1 means totally opaque.
  void SetParam(int font_size, float interval_ratio = 0.1,
                float whitespace_ratio = 0.5, float alpha = 1);

  /// Draw text on an Opencv Mat
  /// \param img
  /// \param utf8_text: Text encoded in utf8. (if it is hardcoded, make sure
  /// your source file is saved
  ///                   with utf8 encoding.
  /// \param org: The left-bottom point of the text. Notice some letters like
  /// 'g' may go under this point \param color \param calc_size: true -> return
  /// rect. False -> return rect and draw text.
  ///                   Useful e.g. when you want to draw a rectangle under the
  ///                   text
  /// \return The precise bounding box of the text in the image
  cv::Rect PutText(cv::Mat& img, const std::string& utf8_text,
                   const cv::Point& org, const cv::Scalar& color,
                   bool calc_size = false);
  bool genBitMap(bm_handle_t mHandle, const std::string& utf8_text,
                 bm_image& overlay_image, int r, int g, int b);

 private:
  /// Hide implementations
  std::unique_ptr<Impl> pimpl;
};
}  // namespace uni_text

#endif
