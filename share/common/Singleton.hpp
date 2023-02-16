#pragma once

namespace sophon_stream {
namespace common {


template<class T>
class Singleton {
public:
    typedef T ObjectType;

    static ObjectType& getInstance() {
        static ObjectType obj;
        return obj;
    }
};

} // namespace common
} // namespace sophon_stream

