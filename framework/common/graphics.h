//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_GRAPHICS_H_
#define SOPHON_STREAM_COMMON_GRAPHICS_H_

#include <algorithm>
#include <vector>

namespace sophon_stream {
namespace common {

template <class T>
struct Point {
  Point() : mX(0), mY(0) {}

  Point(T x, T y) : mX(x), mY(y) {}

  T mX;
  T mY;
};

template <class T>
struct Segment {
  Segment() {}

  Segment(const Point<T>& a, const Point<T>& b) : mA(a), mB(b) {}

  Point<T> mA;
  Point<T> mB;
};

template <class T>
struct Size {
  Size() : mWidth(0), mHeight(0) {}

  Size(T width, T height) : mWidth(width), mHeight(height) {}

  T area() const { return mWidth * mHeight; }

  bool empty() const { return 0 == mWidth || 0 == mHeight; }

  T mWidth;
  T mHeight;
};

template <class T>
struct Rectangle {
  Rectangle() : mX(0), mY(0), mWidth(0), mHeight(0) {}

  Rectangle(T x, T y, T width, T height)
      : mX(x), mY(y), mWidth(width), mHeight(height) {}

  T top() const { return mY; }

  T bottom() const { return mY + mHeight; }

  T left() const { return mX; }

  T right() const { return mX + mWidth; }

  Point<T> center() const {
    return Point<T>(mX + mWidth / 2, mY + mHeight / 2);
  }

  T area() const { return mWidth * mHeight; }

  bool empty() const { return 0 == mWidth || 0 == mHeight; }

  T mX;
  T mY;
  T mWidth;
  T mHeight;
};

template <class T>
struct Polygon {
  std::vector<Point<T> > mPoints;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_GRAPHICS_H_