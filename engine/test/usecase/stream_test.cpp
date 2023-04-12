#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "element/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include "config.h"
#include <fstream>
#include "common/Clocker.h"

#include <opencv2/opencv.hpp>
#include "common/ff_decode.cpp"


struct SophonData{
    SophonData(){
            mData.reset(new bm_device_mem_t, [&](bm_device_mem_t* p){
                std::cout<<" deconstruct data "<<std::endl;
                bm_free_device(mHandle, *p);
                delete p;
        });
    }
    bm_handle_t mHandle;
    std::shared_ptr<bm_device_mem_t> mData;
};


int main()
{
    std::shared_ptr<SophonData> a = std::make_shared<SophonData>();
    bm_dev_request(&a->mHandle,0);
    bm_malloc_device_byte(a->mHandle,a->mData.get(),1024);
    

    VideoDecFFM decoder;
    decoder.openDec(&a->mHandle,"../test/test_car_person_1080P.mp4");
    int eof = 0;
    double timestamp = 0.0;
    while(eof!=1){
        decoder.grab(eof,timestamp);
    }
    std::cout<<" end of stream "<<std::endl;

    return 0;
}

// struct SophonData{
//     SophonData(){}
//     bm_handle_t mHandle;
//     bm_device_mem_t* mData;
//     ~SophonData(){
//     }
// };


// int main()
// {
//     std::shared_ptr<SophonData> a = std::make_shared<SophonData>();
//     bm_dev_request(&a->mHandle,0);
//     a->mData = new bm_device_mem_t;
//     bm_malloc_device_byte(a->mHandle,a->mData,1024);
//     bm_free_device(a->mHandle, *a->mData);

//     return 0;
// }



// int main()
// {
//     bm_handle_t h;
//     bm_dev_request(&h,0);
//     bm_device_mem_t * data = new bm_device_mem_t;
//     bm_malloc_device_byte(h,data,1024);
//     bm_free_device(h, *data);
//     return 0;
// }


// template<typename T>
// class A
// {
//     std::shared_ptr<T> t;
//     A() = default;
//     A(){t = std::make_shared(T);}
// };

// template<typename T>
// void foo(std::shared_ptr<A<T>> & a)
// {
//     a.t.reset(new T, [&](T* p){
//             delete p;
//         });
//     return;
// }

