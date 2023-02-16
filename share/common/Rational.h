#pragma once

namespace sophon_stream {
namespace common {

struct Rational {
    Rational()
        : mNumber(0), 
          mDenominator(1) {
    }

    Rational(int number, int denominator)
        : mNumber(number), 
          mDenominator(denominator) {
    }

    int mNumber;
    int mDenominator;
};

} // namespace common
} // namespace sophon_stream


