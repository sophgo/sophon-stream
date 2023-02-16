/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by yang jun <jun.yang@lynxi.com>, 20-3-17
*/

/**
@file unitcaseTransform.cpp
@brief 主程序
@details
    Transform单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include <gtest/gtest.h>

#include "grpc/Transform.h"
#include "common/Logger.h"

/**
@brief Transform单元测试函数入口

@param [in] TestTransform     测试用例命名
@param [in] ProtobufToStruct  测试命名
@return void 无返回值


@UnitCase_ID
Transform_UT_0011

@UnitCase_Name
unitcaseTransform

@UnitCase_Description
实例化grpc::Packet为protobufPacket，实例化common::Packet为structPacket，初始化protobufPacket，通过ProtobufToStruct方法将ProtobufToStruct转换为structPacket，验证structPacket中的数据是否正确。
实例化grpc::Frame为protobufFrame，实例化common::Frame为structFrame，初始化protobufFrame，通过ProtobufToStruct方法将protobufFrame转换为structFrame，验证structFrame中的数据是否正确。
实例化grpc::ObjectMetadata为protobufObjectMetadata，实例化common::ObjectMetadata为structObjectMetadata，初始化protobufObjectMetadata，通过ProtobufToStruct方法将protobufObjectMetadata转换为structObjectMetadata，验证structObjectMetadata中的数据是否正确。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestTransform, ProtobufToStruct

@UnitCase_ExpectedResult
grpc::Packet、grpc::Frame和grpc::ObjectMetadata能正常实例化，并通过ProtobufToStruct方法分别转换为common::Packet、common::Frame和common::ObjectMetadata，程序能正常退出

*/

TEST(TestTransform, ProtobufToStruct) {
    lynxi::ivs::grpc::Packet protobufPacket;
    lynxi::ivs::common::Packet structPacket;

    protobufPacket.set_channel_id(100); 
    protobufPacket.set_packet_id(1000); 
    protobufPacket.set_codec_type(lynxi::ivs::grpc::Packet::H264);
    protobufPacket.mutable_frame_rate()->set_number(1);
    protobufPacket.mutable_frame_rate()->set_denominator(25);
    protobufPacket.set_timestamp(50); 
    protobufPacket.set_key_frame(true); 
    protobufPacket.set_end_of_stream(true); 
    protobufPacket.set_width(1920); 
    protobufPacket.set_height(1080); 
    protobufPacket.set_data("hello world");

    lynxi::ivs::grpc::ProtobufToStruct("host", 0, protobufPacket, structPacket);

    ASSERT_EQ(100, structPacket.mChannelId);
    ASSERT_EQ(1000, structPacket.mPacketId);
    ASSERT_EQ(lynxi::ivs::common::CodecType::H264, structPacket.mCodecType);
    ASSERT_EQ(1, structPacket.mFrameRate.mNumber);
    ASSERT_EQ(25, structPacket.mFrameRate.mDenominator);
    ASSERT_EQ(50, structPacket.mTimestamp);
    ASSERT_EQ(true, structPacket.mKeyFrame);
    ASSERT_EQ(true, structPacket.mEndOfStream);
    ASSERT_EQ(1920, structPacket.mWidth);
    ASSERT_EQ(1080, structPacket.mHeight);
    ASSERT_EQ("hello world", std::string(static_cast<char*>(structPacket.mData.get()), 
                                         structPacket.mDataSize));


    lynxi::ivs::grpc::Frame protobufFrame;
    lynxi::ivs::common::Frame structFrame;

    protobufFrame.set_channel_id(100); 
    protobufFrame.set_frame_id(1000); 
    protobufFrame.set_format_type(lynxi::ivs::grpc::Frame::RGB_PACKET);
    protobufFrame.set_data_type(lynxi::ivs::grpc::Frame::FLOATING_POINT);
    protobufFrame.mutable_frame_rate()->set_number(1);
    protobufFrame.mutable_frame_rate()->set_denominator(25);
    protobufFrame.set_timestamp(50); 
    protobufFrame.set_end_of_stream(true); 
    protobufFrame.set_channel(1); 
    protobufFrame.set_channel_step(3 * sizeof(float) * 1920 * 1080); 
    protobufFrame.set_width(1920); 
    protobufFrame.set_width_step(3 * sizeof(float)); 
    protobufFrame.set_height(1080); 
    protobufFrame.set_height_step(3 * sizeof(float) * 1920); 
    protobufFrame.set_data("hello world");

    lynxi::ivs::grpc::ProtobufToStruct("host", 0, protobufFrame, structFrame);

    ASSERT_EQ(100, structFrame.mChannelId);
    ASSERT_EQ(1000, structFrame.mFrameId);
    ASSERT_EQ(lynxi::ivs::common::FormatType::RGB_PACKET, structFrame.mFormatType);
    ASSERT_EQ(lynxi::ivs::common::DataType::FLOATING_POINT, structFrame.mDataType);
    ASSERT_EQ(1, structFrame.mFrameRate.mNumber);
    ASSERT_EQ(25, structFrame.mFrameRate.mDenominator);
    ASSERT_EQ(50, structFrame.mTimestamp);
    ASSERT_EQ(true, structFrame.mEndOfStream);
    ASSERT_EQ(1, structFrame.mChannel);
    ASSERT_EQ(3 * sizeof(float) * 1920 * 1080, structFrame.mChannelStep);
    ASSERT_EQ(1920, structFrame.mWidth);
    ASSERT_EQ(3 * sizeof(float), structFrame.mWidthStep);
    ASSERT_EQ(1080, structFrame.mHeight);
    ASSERT_EQ(3 * sizeof(float) * 1920, structFrame.mHeightStep);
    ASSERT_EQ("hello world", std::string(static_cast<char*>(structFrame.mData.get()), 
                                         structFrame.mDataSize));

//    lynxi::ivs::grpc::DetectedObjectMetadata protobufDetectedObjectMetadata;
//    lynxi::ivs::common::DetectedObjectMetadata structDetectedObjectMetadata;
//
//    protobufDetectedObjectMetadata.mutable_box()->set_x(100);
//    protobufDetectedObjectMetadata.mutable_box()->set_y(100);
//    protobufDetectedObjectMetadata.mutable_box()->set_width(200);
//    protobufDetectedObjectMetadata.mutable_box()->set_height(50);
//    protobufDetectedObjectMetadata.set_item_name("test");
//    protobufDetectedObjectMetadata.set_label_name("god");
//    protobufDetectedObjectMetadata.add_scores(0.f);
//    protobufDetectedObjectMetadata.add_scores(99.99f);
//    protobufDetectedObjectMetadata.add_top_k_labels(1);
//    protobufDetectedObjectMetadata.set_classify(0);
//    protobufDetectedObjectMetadata.set_classify_name("inhuman");
//
//    lynxi::ivs::grpc::ProtobufToStruct(protobufDetectedObjectMetadata, structDetectedObjectMetadata);
//
//    ASSERT_EQ(100, structDetectedObjectMetadata.mBox.mX);
//    ASSERT_EQ(100, structDetectedObjectMetadata.mBox.mY);
//    ASSERT_EQ(200, structDetectedObjectMetadata.mBox.mWidth);
//    ASSERT_EQ(50, structDetectedObjectMetadata.mBox.mHeight);
//    ASSERT_EQ("test", structDetectedObjectMetadata.mItemName);
//    ASSERT_EQ("god", structDetectedObjectMetadata.mLabelName);
//    ASSERT_EQ(2, structDetectedObjectMetadata.mScores.size());
//    ASSERT_GT(0.01f, structDetectedObjectMetadata.mScores[0]);
//    ASSERT_LT(99.9f, structDetectedObjectMetadata.mScores[1]);
//    ASSERT_EQ(1, structDetectedObjectMetadata.mTopKLabels.size());
//    ASSERT_EQ(1, structDetectedObjectMetadata.mTopKLabels[0]);
//    ASSERT_EQ(0, structDetectedObjectMetadata.mClassify);
//    ASSERT_EQ("inhuman", structDetectedObjectMetadata.mClassifyName);
//    ASSERT_EQ(0, structDetectedObjectMetadata.mKeyPoints.size());
//
//    lynxi::ivs::grpc::RecognizedObjectMetadata protobufRecognizedObjectMetadata;
//    lynxi::ivs::common::RecognizedObjectMetadata structRecognizedObjectMetadata;
//
//    protobufRecognizedObjectMetadata.set_item_name("test");
//    protobufRecognizedObjectMetadata.set_label_name("god");
//    protobufRecognizedObjectMetadata.add_scores(0.f);
//    protobufRecognizedObjectMetadata.add_scores(99.99f);
//    protobufRecognizedObjectMetadata.add_top_k_labels(1);
//
//    lynxi::ivs::grpc::ProtobufToStruct(protobufRecognizedObjectMetadata, structRecognizedObjectMetadata);
//
//    ASSERT_EQ("test", structRecognizedObjectMetadata.mItemName);
//    ASSERT_EQ("god", structRecognizedObjectMetadata.mLabelName);
//    ASSERT_EQ(2, structRecognizedObjectMetadata.mScores.size());
//    ASSERT_GT(0.01f, structRecognizedObjectMetadata.mScores[0]);
//    ASSERT_LT(99.9f, structRecognizedObjectMetadata.mScores[1]);
//    ASSERT_EQ(1, structRecognizedObjectMetadata.mTopKLabels.size());
//    ASSERT_EQ(1, structRecognizedObjectMetadata.mTopKLabels[0]);

    lynxi::ivs::grpc::ObjectMetadata protobufObjectMetadata;
    lynxi::ivs::common::ObjectMetadata structObjectMetadata;

    (*protobufObjectMetadata.mutable_packet()) = protobufPacket;
    (*protobufObjectMetadata.mutable_frame()) = protobufFrame;
    (*protobufObjectMetadata.mutable_model_configure_map())["detect"] = true;
    (*protobufObjectMetadata.mutable_model_configure_map())["recognize"] = false;

    lynxi::ivs::grpc::ObjectMetadata protobufSubObjectMetadata;
//    (*protobufSubObjectMetadata.mutable_detected_object_metadata()) = protobufDetectedObjectMetadata;
//    (*protobufSubObjectMetadata.add_recognized_object_metadatas()) = protobufRecognizedObjectMetadata;
    (*protobufObjectMetadata.add_sub_object_metadatas()) = protobufSubObjectMetadata;

    lynxi::ivs::grpc::ProtobufToStruct("host", 0, protobufObjectMetadata, structObjectMetadata);

    ASSERT_EQ(1, structObjectMetadata.mSubObjectMetadatas.size());
    ASSERT_EQ(structObjectMetadata.mSubObjectMetadatas[0]->mPacket, structObjectMetadata.mPacket);
    ASSERT_EQ(structObjectMetadata.mSubObjectMetadatas[0]->mFrame, structObjectMetadata.mFrame);
    ASSERT_EQ(structObjectMetadata.mSubObjectMetadatas[0]->mModelConfigureMap, structObjectMetadata.mModelConfigureMap);
    //ASSERT_EQ(1, structObjectMetadata.mSubObjectMetadatas[0]->mRecognizedObjectMetadatas.size());
}

/**
@brief Transform单元测试函数入口

@param [in] TestTransform     测试用例命名
@param [in] ProtobufToStruct  测试命名
@return void 无返回值


@UnitCase_ID
Transform_UT_0012

@UnitCase_Name
unitcaseTransform

@UnitCase_Description
实例化grpc::Packet为protobufPacket，实例化common::Packet为structPacket，初始化protobufPacket，通过ProtobufToStruct方法将ProtobufToStruct转换为structPacket，验证structPacket中的数据是否正确。
实例化grpc::Frame为protobufFrame，实例化common::Frame为structFrame，初始化protobufFrame，通过ProtobufToStruct方法将protobufFrame转换为structFrame，验证structFrame中的数据是否正确。
实例化grpc::ObjectMetadata为protobufObjectMetadata，实例化common::ObjectMetadata为structObjectMetadata，初始化protobufObjectMetadata，通过ProtobufToStruct方法将protobufObjectMetadata转换为structObjectMetadata，验证structObjectMetadata中的数据是否正确。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestTransform, ProtobufToStruct

@UnitCase_ExpectedResult
grpc::Packet、grpc::Frame和grpc::ObjectMetadata能正常实例化，并通过ProtobufToStruct方法分别转换为common::Packet、common::Frame和common::ObjectMetadata，程序能正常退出

*/

TEST(TestTransform, StructToProtobuf) {
    lynxi::ivs::grpc::Packet protobufPacket;
    lynxi::ivs::common::Packet structPacket;

    structPacket.mChannelId = 101;
    structPacket.mPacketId = 1001;
    structPacket.mCodecType = lynxi::ivs::common::CodecType::HEVC;
    structPacket.mFrameRate.mNumber = 1;
    structPacket.mFrameRate.mDenominator = 5;
    structPacket.mTimestamp = 10;
    structPacket.mKeyFrame = true;
    structPacket.mEndOfStream = true;
    structPacket.mWidth = 4096;
    structPacket.mHeight = 2048;
    structPacket.mDataSize = 11;
    structPacket.mData = std::static_pointer_cast<void>(std::shared_ptr<char>(new char[structPacket.mDataSize], 
                                                                              [](char* data) {
                                                                                  delete [] data;
                                                                              }));
    std::memcpy(structPacket.mData.get(), "hello\0world", structPacket.mDataSize);

    lynxi::ivs::grpc::StructToProtobuf(structPacket, protobufPacket);

    ASSERT_EQ(101, protobufPacket.channel_id());
    ASSERT_EQ(1001, protobufPacket.packet_id());
    ASSERT_EQ(lynxi::ivs::grpc::Packet::H265, protobufPacket.codec_type());
    ASSERT_EQ(1, protobufPacket.frame_rate().number());
    ASSERT_EQ(5, protobufPacket.frame_rate().denominator());
    ASSERT_EQ(10, protobufPacket.timestamp());
    ASSERT_EQ(true, protobufPacket.key_frame());
    ASSERT_EQ(true, protobufPacket.end_of_stream());
    ASSERT_EQ(4096, protobufPacket.width());
    ASSERT_EQ(2048, protobufPacket.height());
    ASSERT_EQ(0, std::memcmp("hello\0world", protobufPacket.data().data(), protobufPacket.data().size()));

    lynxi::ivs::grpc::Frame protobufFrame;
    lynxi::ivs::common::Frame structFrame;

    structFrame.mChannelId = 101;
    structFrame.mFrameId = 1001;
    structFrame.mFormatType = lynxi::ivs::common::FormatType::NV12;
    structFrame.mDataType = lynxi::ivs::common::DataType::INTEGER;
    structFrame.mFrameRate.mNumber = 1;
    structFrame.mFrameRate.mDenominator = 5;
    structFrame.mTimestamp = 10;
    structFrame.mEndOfStream = true;
    structFrame.mChannel = 2;
    structFrame.mChannelStep = 4096 * 2048;
    structFrame.mWidth = 4096;
    structFrame.mWidthStep = 1;
    structFrame.mHeight = 2048;
    structFrame.mHeightStep = 4096;
    structFrame.mDataSize = 11;
    structFrame.mData = std::static_pointer_cast<void>(std::shared_ptr<char>(new char[structFrame.mDataSize], 
                                                                             [](char* data) {
                                                                                 delete [] data;
                                                                             }));
    std::memcpy(structFrame.mData.get(), "hello\0world", structFrame.mDataSize);

    lynxi::ivs::grpc::StructToProtobuf(structFrame, protobufFrame);

    ASSERT_EQ(101, protobufFrame.channel_id());
    ASSERT_EQ(1001, protobufFrame.frame_id());
    ASSERT_EQ(lynxi::ivs::grpc::Frame::NV12, protobufFrame.format_type());
    ASSERT_EQ(lynxi::ivs::grpc::Frame::INTEGER, protobufFrame.data_type());
    ASSERT_EQ(1, protobufFrame.frame_rate().number());
    ASSERT_EQ(5, protobufFrame.frame_rate().denominator());
    ASSERT_EQ(10, protobufFrame.timestamp());
    ASSERT_EQ(true, protobufFrame.end_of_stream());
    ASSERT_EQ(2, protobufFrame.channel());
    ASSERT_EQ(4096 * 2048, protobufFrame.channel_step());
    ASSERT_EQ(4096, protobufFrame.width());
    ASSERT_EQ(1, protobufFrame.width_step());
    ASSERT_EQ(2048, protobufFrame.height());
    ASSERT_EQ(4096, protobufFrame.height_step());
    ASSERT_EQ(0, std::memcmp("hello\0world", protobufFrame.data().data(), protobufFrame.data().size()));

//    lynxi::ivs::grpc::DetectedObjectMetadata protobufDetectedObjectMetadata;
//    lynxi::ivs::common::DetectedObjectMetadata structDetectedObjectMetadata;
//
//    structDetectedObjectMetadata.mBox.mX = 1;
//    structDetectedObjectMetadata.mBox.mY = 1;
//    structDetectedObjectMetadata.mBox.mWidth = 1918;
//    structDetectedObjectMetadata.mBox.mHeight = 1078;
//    structDetectedObjectMetadata.mItemName = "test";
//    structDetectedObjectMetadata.mLabelName = "et";
//    structDetectedObjectMetadata.mScores.push_back(89.99f);
//    structDetectedObjectMetadata.mScores.push_back(10.01f);
//    structDetectedObjectMetadata.mTopKLabels.push_back(0);
//    structDetectedObjectMetadata.mClassify = 0;
//    structDetectedObjectMetadata.mClassifyName = "inhuman";
//
//    lynxi::ivs::grpc::StructToProtobuf(structDetectedObjectMetadata, protobufDetectedObjectMetadata);
//
//    ASSERT_EQ(1, protobufDetectedObjectMetadata.box().x());
//    ASSERT_EQ(1, protobufDetectedObjectMetadata.box().y());
//    ASSERT_EQ(1918, protobufDetectedObjectMetadata.box().width());
//    ASSERT_EQ(1078, protobufDetectedObjectMetadata.box().height());
//    ASSERT_EQ("test", protobufDetectedObjectMetadata.item_name());
//    ASSERT_EQ("et", protobufDetectedObjectMetadata.label_name());
//    ASSERT_EQ(2, protobufDetectedObjectMetadata.scores().size());
//    ASSERT_GT(90.f, protobufDetectedObjectMetadata.scores(0));
//    ASSERT_LT(89.f, protobufDetectedObjectMetadata.scores(0));
//    ASSERT_GT(11.f, protobufDetectedObjectMetadata.scores(1));
//    ASSERT_LT(10.f, protobufDetectedObjectMetadata.scores(1));
//    ASSERT_EQ(1, protobufDetectedObjectMetadata.top_k_labels().size());
//    ASSERT_EQ(0, protobufDetectedObjectMetadata.top_k_labels(0));
//    ASSERT_EQ(0, protobufDetectedObjectMetadata.classify());
//    ASSERT_EQ("inhuman", protobufDetectedObjectMetadata.classify_name());
//    ASSERT_EQ(0, protobufDetectedObjectMetadata.key_points().size());
//
//    lynxi::ivs::grpc::RecognizedObjectMetadata protobufRecognizedObjectMetadata;
//    lynxi::ivs::common::RecognizedObjectMetadata structRecognizedObjectMetadata;
//
//    structRecognizedObjectMetadata.mItemName = "test";
//    structRecognizedObjectMetadata.mLabelName = "et";
//    structRecognizedObjectMetadata.mScores.push_back(89.99f);
//    structRecognizedObjectMetadata.mScores.push_back(10.01f);
//    structRecognizedObjectMetadata.mTopKLabels.push_back(0);
//
//    lynxi::ivs::grpc::StructToProtobuf(structRecognizedObjectMetadata, protobufRecognizedObjectMetadata);

//    ASSERT_EQ("test", protobufRecognizedObjectMetadata.item_name());
//    ASSERT_EQ("et", protobufRecognizedObjectMetadata.label_name());
//    ASSERT_EQ(2, protobufRecognizedObjectMetadata.scores().size());
//    ASSERT_GT(90.f, protobufRecognizedObjectMetadata.scores(0));
//    ASSERT_LT(89.f, protobufRecognizedObjectMetadata.scores(0));
//    ASSERT_GT(11.f, protobufRecognizedObjectMetadata.scores(1));
//    ASSERT_LT(10.f, protobufRecognizedObjectMetadata.scores(1));
//    ASSERT_EQ(1, protobufRecognizedObjectMetadata.top_k_labels().size());
//    ASSERT_EQ(0, protobufRecognizedObjectMetadata.top_k_labels(0));
//    ASSERT_EQ(0, protobufRecognizedObjectMetadata.top_k_label_metadatas().size());

    lynxi::ivs::grpc::ObjectMetadata protobufObjectMetadata;
    lynxi::ivs::common::ObjectMetadata structObjectMetadata;

    structObjectMetadata.mPacket = std::make_shared<lynxi::ivs::common::Packet>();
    *structObjectMetadata.mPacket = structPacket;
    structObjectMetadata.mFrame = std::make_shared<lynxi::ivs::common::Frame>();
    *structObjectMetadata.mFrame = structFrame;
    structObjectMetadata.mModelConfigureMap = std::make_shared<lynxi::ivs::common::ModelConfigureMap>();
    (*structObjectMetadata.mModelConfigureMap)["detect"] = true;
    (*structObjectMetadata.mModelConfigureMap)["recognize"] = false;

    auto structSubObjectMetadata = std::make_shared<lynxi::ivs::common::ObjectMetadata>();
//    structSubObjectMetadata->mDetectedObjectMetadata = std::make_shared<lynxi::ivs::common::DetectedObjectMetadata>();
//    *structSubObjectMetadata->mDetectedObjectMetadata = structDetectedObjectMetadata;
//    structSubObjectMetadata->mRecognizedObjectMetadatas.push_back(std::make_shared<lynxi::ivs::common::RecognizedObjectMetadata>(structRecognizedObjectMetadata));
//    structObjectMetadata.mSubObjectMetadatas.push_back(structSubObjectMetadata);

    lynxi::ivs::grpc::StructToProtobuf(structObjectMetadata, protobufObjectMetadata);

    ASSERT_EQ(1, protobufObjectMetadata.sub_object_metadatas().size());
    ASSERT_EQ(false, protobufObjectMetadata.sub_object_metadatas(0).has_packet());
    ASSERT_EQ(false, protobufObjectMetadata.sub_object_metadatas(0).has_frame());
    ASSERT_EQ(false, protobufObjectMetadata.sub_object_metadatas(0).model_configure_map().size());
    ASSERT_EQ(1, protobufObjectMetadata.sub_object_metadatas(0).recognized_object_metadatas().size());
}
