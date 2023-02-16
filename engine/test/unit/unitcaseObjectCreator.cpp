//#include "common/ObjectCreator.hpp"
//#include "gtest/gtest.h"

////gtest的这些TEST()都在一个进程中，所以ObjectCreator::getInstance()在这里适合调一次

//TEST(TestObjectCreator, createObject) {
//    auto &obj = ObjectCreator::getInstance();

//    int testNum = 5;
//    obj.registerClass("abc", [&](void)->void * {
//        return &testNum;
//    });

//    void *p = obj.createObject("abc");

//    ASSERT_EQ(*(int *)p, 5);

//    ASSERT_EQ(sizeof(obj), 48);

//}

//TEST(TestObjectCreator, getInstance) {
//    auto &obj = ObjectCreator::getInstance();
//    ASSERT_EQ(sizeof(obj), 48);
//}

//TEST(TestObjectCreator, registerClass) {
//    auto &obj = ObjectCreator::getInstance();

//    obj.registerClass("abc", [&](void)->void * {
//        return nullptr;
//    });

//    ASSERT_EQ(sizeof(obj), 48);
//}



