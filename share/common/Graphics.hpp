#pragma once

#include <algorithm>
#include <vector>

namespace sophon_stream {
namespace common {

template <class T>
struct Point {
    Point()
        : mX(0), 
          mY(0) {
    }

    Point(T x, T y)
        : mX(x), 
          mY(y) {
    }

    T mX;
    T mY;
};

template <class T>
bool operator ==(const Point<T>& a, const Point<T>& b) {
    return a.mX == b.mX
        && a.mY == b.mY;
}

template <class T>
bool operator !=(const Point<T>& a, const Point<T>& b) {
    return !(a == b);
}

template <class T>
struct Segment {
    Segment() {
    }

    Segment(const Point<T>& a,  const Point<T>& b)
        : mA(a), 
          mB(b) {
    }

    Point<T> mA;
    Point<T> mB;
};

template <class T>
bool operator ==(const Segment<T>& a, const Segment<T>& b) {
    return a.mA == b.mA
        && a.mB == b.mB;
}

template <class T>
bool operator !=(const Segment<T>& a, const Segment<T>& b) {
    return !(a == b);
}

template <class T>
bool pointOverSegment(const Point<T>& point, const Segment<T>& segment) {
    const auto& left = segment.mA.mX < segment.mB.mX ? segment.mA : segment.mB;
    const auto& right = segment.mA.mX < segment.mB.mX ? segment.mB : segment.mA;

    return point.mX >= left.mX && point.mX <= right.mX
        && (right.mY - point.mY) * (point.mX - left.mX) > (point.mY - left.mY) * (right.mX - point.mX);
}

template <class T>
bool pointUnderSegment(const Point<T>& point, const Segment<T>& segment) {
    const auto& left = segment.mA.mX < segment.mB.mX ? segment.mA : segment.mB;
    const auto& right = segment.mA.mX < segment.mB.mX ? segment.mB : segment.mA;

    return point.mX >= left.mX && point.mX <= right.mX
        && (point.mX - left.mY) * (right.mX - point.mX) > (right.mY - point.mY) * (point.mX - left.mX);
}

template <class T>
struct Size {
    Size()
        : mWidth(0), 
          mHeight(0) {
    }

    Size(T width, T height)
        : mWidth(width), 
          mHeight(height) {
    }

    T area() const {
        return mWidth * mHeight;
    }

    bool empty() const {
        return 0 == mWidth
            || 0 == mHeight;
    }
    
    T mWidth;
    T mHeight;
};

template <class T>
bool operator ==(const Size<T>& a, const Size<T>& b) {
    return a.mWidth == b.mWidth
        && a.mHeight == b.mHeight;
}

template <class T>
bool operator !=(const Size<T>& a, const Size<T>& b) {
    return !(a == b);
}

template <class T>
struct Rectangle {
    Rectangle()
        : mX(0), 
          mY(0), 
          mWidth(0), 
          mHeight(0) {
    }

    Rectangle(T x, T y, T width, T height)
        : mX(x), 
          mY(y), 
          mWidth(width), 
          mHeight(height) {
    }

    T top() const { 
        return mY; 
    }

    T bottom() const { 
        return mY + mHeight; 
    }

    T left() const { 
        return mX; 
    }

    T right() const { 
        return mX + mWidth; 
    }

    Point<T> center() const {
        return Point<T>(mX + mWidth / 2, mY + mHeight / 2);
    }

    T area() const {
        return mWidth * mHeight;
    }

    bool empty() const {
        return 0 == mWidth
            || 0 == mHeight;
    }

    T mX;
    T mY;
    T mWidth;
    T mHeight;
};

template <class T>
bool operator ==(const Rectangle<T>&a, const Rectangle<T>& b) {
    return a.mX == b.mX
        && a.mY == b.mY
        && a.mWidth == b.mWidth
        && a.mHeight == b.mHeight;
}

template <class T>
bool operator !=(const Rectangle<T>&a, const Rectangle<T>& b) {
    return !(a == b);
}

template <class T>
Rectangle<T>& operator &=(Rectangle<T>& a, const Rectangle<T>& b) {
    auto right = std::max(a.right(), b.right());
    auto top = std::max(a.top(), b.top());

    a.mWidth = std::min(a.left(), b.left()) - right;
    a.mHeight = std::min(a.bottom(), b.bottom()) - top;
    a.mX = right;
    a.mY = top;

    return a;
}

template <class T>
Rectangle<T> operator &(const Rectangle<T>& a, const Rectangle<T>& b) {
    auto c = a;
    return c &= b;
}

template <class T>
bool pointInRectangle(const Point<T>& point, const Rectangle<T>& rectangle) {
    return point.mX >= rectangle.left()
        && point.mX <= rectangle.right()
        && point.mY >= rectangle.top()
        && point.mY <= rectangle.bottom();
}

template <class T>
struct Polygon {
    std::vector<Point<T> > mPoints;
};

template <class T>
bool operator ==(const Polygon<T>& a, const Polygon<T>& b) {
    return a.mPoints == b.mPoints;
}

template <class T>
bool operator !=(const Polygon<T>& a, const Polygon<T>& b) {
    return !(a == b);
}

template <class T>
bool pointInPolygon(const Point<T>& point, const Polygon<T>& polygon) {
    int pointOverSegmentCount = 0;
    for (int i = 0; i < polygon.mPoints.size(); ++i) {
        const auto& a = polygon.mPoints[i];
        const auto& b = polygon.mPoints[(i + 1) % polygon.mPoints.size()];
        const auto& left = a.mX < b.mX ? a : b;
        if (pointOverSegment(point, Segment<T>(a, b))
                && point.mX != left.mX) {
            ++pointOverSegmentCount;
        }
    }

    return 0 != pointOverSegmentCount % 2;
}

} // namespace common
} // namespace sophon_stream

