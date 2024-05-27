#include "cvUniText.h"

#include <freetype/freetype.h>
#include <ft2build.h>

#include <cstdio>

#include "utf8.h"

using namespace uni_text;

namespace uni_text {
class Impl {
 public:
  Impl(const std::string& font_face, int font_size);

  ~Impl();

  void SetParam(int font_size, float interval_ratio = 0.1,
                float whitespace_ratio = 0.3, float alpha = 1);

  cv::Rect PutText(cv::Mat& img, const std::string& text, const cv::Point& org,
                   const cv::Scalar& color, bool calc_size);
  bool genBitMap(bm_handle_t mHandle, const std::string& utf8_text,
                 bm_image& overlay_image, int r, int g, int b);

 private:
  cv::Rect _cvPutUniTextUCS2(cv::Mat& img, const std::u16string& text,
                             const cv::Point& org, const cv::Scalar& color,
                             bool calc_size);

  double _cvPutUniChar(cv::Mat& img, char16_t wc, const cv::Point& pos,
                       const cv::Scalar& color, bool calc_size);

  FT_Library m_library;
  FT_Face m_face;
  int m_fontType;
  cv::Scalar m_fontSize;
  float m_fontDiaphaneity;
};
}  // namespace uni_text

UniText::UniText(const std::string& font_face, int font_size) {
  pimpl = std::unique_ptr<Impl>(new Impl(font_face, font_size));
}

UniText::~UniText() = default;

void UniText::SetParam(int font_size, float interval_ratio,
                       float whitespace_ratio, float alpha) {
  pimpl->SetParam(font_size, interval_ratio, whitespace_ratio, alpha);
}

cv::Rect UniText::PutText(cv::Mat& img, const std::string& text,
                          const cv::Point& org, const cv::Scalar& color,
                          bool calc_size) {
  return pimpl->PutText(img, text, org, color, calc_size);
}
bool UniText::genBitMap(bm_handle_t mHandle, const std::string& utf8_text,
                        bm_image& overlay_image, int r, int g, int b) {
  return pimpl->genBitMap(mHandle, utf8_text, overlay_image, r, g, b);
}

Impl::Impl(const std::string& font_face, int font_size) {
  if (FT_Init_FreeType(&m_library) != 0) {
    fprintf(stderr, "Freetype init failed!\n");
    abort();
  }

  if (FT_New_Face(m_library, font_face.c_str(), 0, &m_face) != 0) {
    fprintf(stderr, "Freetype font load failed!\n");
    abort();
  }

  m_fontType = 0;
  m_fontSize[0] = font_size;  // FontSize
  m_fontSize[1] = 0.5;        // whitechar ratio, such like ' '
  m_fontSize[2] = 0.1;        // inverval ratio, for each char.
  m_fontDiaphaneity = 1;      // alpha

  FT_Set_Pixel_Sizes(m_face, (int)m_fontSize[0], 0);
}

Impl::~Impl() {
  FT_Done_Face(m_face);
  FT_Done_FreeType(m_library);
}

void Impl::SetParam(int font_size, float interval_ratio, float whitespace_ratio,
                    float alpha) {
  m_fontSize[0] = font_size;         // FontSize
  m_fontSize[1] = whitespace_ratio;  // whitechar ratio, such like ' '
  m_fontSize[2] = interval_ratio;    // inverval ratio, for each char.
  m_fontDiaphaneity = alpha;         // alpha
  FT_Set_Pixel_Sizes(m_face, (int)m_fontSize[0], 0);
}

double Impl::_cvPutUniChar(cv::Mat& img, char16_t wc, const cv::Point& pos,
                           const cv::Scalar& color, bool calc_size) {
  // generate font bitmap from unicode
  FT_GlyphSlot ft_slot;
  //    int ft_ascender;
  int ft_bmp_height;
  int ft_bmp_width;  // 0 when ' '
  int ft_bmp_step;
  int ft_bmp_left;
  int ft_bmp_top;
  unsigned char* ft_bmp_buffer;

  double whitespace_width;
  double interval_width;
  double horizontal_offset;

  //
  // img coordinate
  //  0------+ x
  //  |
  //  +
  //  y
  //
  // freetype bmp coordinate
  //  y
  //  +
  //  |
  //  0------+ x
  //
  // freetype algin
  //  'a'
  //               -+-        -+-
  //                |          |
  //                |height    |top
  //                |          |
  //  -baseline-:  -+-        -+-
  //
  //  'g'
  //               -+-        -+-
  //                |          |
  //                |          |top
  //                |          |
  //  -baseline-:   |height   -+-
  //                |
  //               -+-
  //

  // get font_id from database of char
  //    ft_ascender = m_face->size->metrics.ascender / 64;

  FT_UInt glyph_index = FT_Get_Char_Index(m_face, wc);
  // load bitmap font to slot
  FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
  // render to 8bits
  FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
  ft_slot = m_face->glyph;
  ft_bmp_width = ft_slot->bitmap.width;
  ft_bmp_height = ft_slot->bitmap.rows;
  ft_bmp_step = ft_slot->bitmap.pitch;
  ft_bmp_buffer = ft_slot->bitmap.buffer;
  ft_bmp_left = ft_slot->bitmap_left;
  ft_bmp_top = ft_slot->bitmap_top;

#ifdef CVUNITEXT_DEBUG
  if (wc < 256) {
    printf(" %c: ", ((char*)&wc)[0]);
  } else {
    printf(" 0x%02x%02x: ", ((char*)&wc)[1] & 0xFF, ((char*)&wc)[0] & 0xFF);
  }
  printf("width %4d, height %4d, ", ft_bmp_width, ft_bmp_height);
  printf("left %4d, ", ft_bmp_left);
  printf("top %4d, ", ft_bmp_top);
  printf("\n");
#endif

  // calculate char width
  whitespace_width = m_fontSize[0] * m_fontSize[1];
  interval_width = m_fontSize[0] * m_fontSize[2];
  if (ft_bmp_width != 0) {
    horizontal_offset = ft_bmp_width + interval_width;
  } else {
    horizontal_offset = whitespace_width;
  }

  int loc_x = pos.x + ft_bmp_left;
  int loc_y = pos.y + ft_bmp_height - ft_bmp_top;

  if (calc_size) {
    return horizontal_offset;
  }

  // draw font bitmap to opencv image
  //(bmp_j,bmp_i) is freetype bitmap location
  for (int bmp_i = 0; bmp_i < ft_bmp_height; ++bmp_i) {
    for (int bmp_j = 0; bmp_j < ft_bmp_width; ++bmp_j) {
      int bmp_valoff = bmp_i * ft_bmp_step + bmp_j;
      unsigned int bmp_val = ft_bmp_buffer[bmp_valoff];
      float bmp_valf = (float)bmp_val / 255 * m_fontDiaphaneity;
      if (bmp_val == 0) {
        continue;
      }
      //(img_x,img_y) is opencv bitmap location
      int img_y = loc_y - (ft_bmp_height - 1 - bmp_i);
      int img_x = loc_x + bmp_j;

      // update pixel when location is valid
      //   alpha = font_bitmap_val / 255;
      //   pixel = alpha * color + (1 - alpha) * pixel;
      if ((0 <= img_y) && (img_y < img.rows) && (0 <= img_x) &&
          (img_x < img.cols)) {
        unsigned char* data = img.ptr<unsigned char>(img_y);
        for (int img_channel = 0; img_channel < img.channels(); img_channel++) {
          data[img_x * img.channels() + img_channel] =
              (1 - bmp_valf) * data[img_x * img.channels() + img_channel] +
              bmp_valf * color[img_channel];
        }
      }
    }
  }

  return horizontal_offset;
}

cv::Rect Impl::_cvPutUniTextUCS2(cv::Mat& img, const std::u16string& text,
                                 const cv::Point& org, const cv::Scalar& color,
                                 bool calc_size) {
  cv::Point pt0 = org;
  cv::Point pt1 = org;
  double offset;
  cv::Rect rect;
  int ascender = m_face->size->metrics.ascender / 64;
  int descender = m_face->size->metrics.descender / 64;

  for (unsigned int i = 0; i < text.size(); i++) {
    offset = _cvPutUniChar(img, text[i], pt1, color, calc_size);
    pt1.x += (int)offset;
  }
  rect.width = pt1.x - pt0.x;
  rect.height = ascender - descender;
  rect.x = pt0.x;
  rect.y = pt0.y - ascender;
  return rect;
}

cv::Rect Impl::PutText(cv::Mat& img, const std::string& text,
                       const cv::Point& org, const cv::Scalar& color,
                       bool calc_size) {
  //    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>
  //    convert; std::u16string dest = convert.from_bytes(text);
  std::u16string dest;
  utf8::utf8to32(text.begin(), text.end(), std::back_inserter(dest));
  cv::Rect sz = _cvPutUniTextUCS2(img, dest, org, color, calc_size);
  return sz;
}

bool Impl::genBitMap(bm_handle_t mHandle, const std::string& utf8_text,
                     bm_image& overlay_image, int r, int g, int b) {
  std::u16string dest;
  utf8::utf8to32(utf8_text.begin(), utf8_text.end(), std::back_inserter(dest));

  FT_Pos total_width = 0;
  FT_Pos max_height = 0;
  FT_Pos base_y_offset = 0;

  // 第一次遍历是为了计算总宽度和最大高度
  for (unsigned int i = 0; i < dest.size(); i++) {
    char16_t ch = dest[i];
    // 注意：这里使用FT_Get_Char_Index和FT_Load_Glyph组合来支持多字节字符
    FT_UInt glyph_index = FT_Get_Char_Index(m_face, ch);
    if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT)) {
      IVS_ERROR("Could not load glyph");
      continue;
    }
    total_width += (m_face->glyph->advance.x >> 6);
    max_height =
        std::max(max_height, static_cast<FT_Pos>(m_face->glyph->bitmap.rows));
    base_y_offset =
        std::max(base_y_offset, static_cast<FT_Pos>(m_face->glyph->bitmap_top));
  }

  std::vector<unsigned char> finalBuffer(max_height * total_width * 4,
                                         0);  // 初始化为0
  FT_Pos x_offset = 0;
  // total_width=FFALIGN(total_width,16);
  for (unsigned int i = 0; i < dest.size(); i++) {
    char16_t ch = dest[i];
    FT_UInt glyph_index = FT_Get_Char_Index(m_face, ch);
    if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER)) {
      IVS_ERROR("Could not load glyph");
      continue;
    }
    FT_Bitmap* bitmap = &m_face->glyph->bitmap;
    std::vector<unsigned char> buffer;
    buffer.resize(bitmap->rows * bitmap->width * 4);
    for (unsigned int i = 0; i < bitmap->rows; ++i) {
      for (unsigned int j = 0; j < bitmap->width; ++j) {
        // 计算buffer中的索引位置
        size_t index = (i * bitmap->width + j) * 4;
        unsigned char alpha = bitmap->buffer[i * bitmap->pitch + j];
        // 设置ARGB8888像素值
        buffer[index] = b;          // Blue
        buffer[index + 1] = g;      // Green
        buffer[index + 2] = r;      // Red
        buffer[index + 3] = alpha;  // Alpha
      }
    }

    FT_Pos y_offset_char = base_y_offset - m_face->glyph->bitmap_top;
    for (unsigned int i = 0; i < m_face->glyph->bitmap.rows; ++i) {
      for (unsigned int j = 0; j < m_face->glyph->bitmap.width; ++j) {
        size_t index = (i * m_face->glyph->bitmap.width + j) * 4;
        size_t final_index =
            ((y_offset_char + i) * total_width + x_offset + j) * 4;
        // 确保我们不会超出最终缓冲区的界限
        if (final_index < finalBuffer.size()) {
          std::copy(buffer.begin() + index, buffer.begin() + index + 4,
                    finalBuffer.begin() + final_index);
        }
      }
    }

    // 更新x_offset，确保下一个字符在正确的位置开始
    x_offset += (m_face->glyph->advance.x >> 6);
  }
  int overlay_height = max_height;
  int overlay_width = total_width;
  if (!overlay_height || !overlay_width) return false;

  bm_image overlay_image2;
  bm_image_create(mHandle, overlay_height, overlay_width, FORMAT_ARGB_PACKED,
                  DATA_TYPE_EXT_1N_BYTE, &overlay_image2, NULL);
  int ret = bm_image_alloc_dev_mem(overlay_image2, BMCV_HEAP_ANY);
  STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")                                     

  void* image_data = static_cast<void*>(finalBuffer.data());
  bm_image_copy_host_to_device(overlay_image2, &image_data);

  int stride[3];
  stride[0] = FFALIGN(overlay_width * 4, 16);
  bm_image_create(mHandle, overlay_height, overlay_width, FORMAT_ARGB_PACKED,
                  DATA_TYPE_EXT_1N_BYTE, &overlay_image, stride);
  bmcv_width_align(mHandle, overlay_image2, overlay_image);
  bm_image_destroy(overlay_image2);
  return true;
}