///*
// Copyright (c) 2018 LynxiTech Inc - All rights reserved.

// NOTICE: All information contained here is, and remains
// the property of LynxiTech Incorporation. This file can not
// be copied or distributed without permission of LynxiTech Inc.

// Author: written by Bin Jiang <bin.jiang@lynxi.com>, 18-10-12

// NOTE: unit test for tracker

// Change History:
//   1. Init.
//*/

//#include "gtest/gtest.h"
//#include "plugins/faceRecognition/algorithms/tracker/Tracker.h"
//#include "plugins/faceRecognition/common/Frame.h"

//using namespace ivs;

////test tracker's constructor member function
//TEST(TestTracker, DefaultConstructor) {

//    IouTracker tracker;

//// check tracker's default settings
//    EXPECT_FLOAT_EQ(tracker.getIouThreshold(), 0.2);
//    EXPECT_EQ(tracker.getFrontRatio(), 15);
//    EXPECT_EQ(tracker.getSharpnessVariance(), 50);
//    EXPECT_EQ(tracker.getUntrackThreshold(), 5);
//    EXPECT_EQ(tracker.getFaceListMaxLen(), 10);
//}

//// test tracker's setFaceListMaxLen member function
//TEST(TestSet, FaceListMaxLen) {

//    IouTracker tracker;

//    tracker.setFaceListMaxLen(100);
//    EXPECT_EQ(tracker.getFaceListMaxLen(), 100);
//    tracker.setFaceListMaxLen(-123);
//    EXPECT_EQ(tracker.getFaceListMaxLen(), 100);
//}

//// test tracker's setFrontRatio member function
//TEST(TestSet, FrontRatio) {

//    IouTracker tracker;

//    tracker.setFrontRatio(101);
//    EXPECT_EQ(tracker.getFrontRatio(), 101);
//    tracker.setFrontRatio(-123);
//    EXPECT_EQ(tracker.getFrontRatio(), 101);
//}

//// test tracker's setIouThreshold member function
//TEST(TestSet, IouThreshold) {

//    IouTracker tracker;

//    tracker.setIouThreshold(102);
//    EXPECT_EQ(tracker.getIouThreshold(), 102);
//    tracker.setIouThreshold(-123);
//    EXPECT_EQ(tracker.getIouThreshold(), 102);
//}


//// test tracker's setSharpnessVariance member function
//TEST(TestSet, SharpnessVariance) {

//    IouTracker tracker;

//    tracker.setSharpnessVariance(103);
//    EXPECT_EQ(tracker.getSharpnessVariance(), 103);
//    tracker.setSharpnessVariance(-123);
//    EXPECT_EQ(tracker.getSharpnessVariance(), 103);
//}

//// test tracker's setUntrackThreshold member function
//TEST(TestSet, UntrackThreshold) {

//    IouTracker tracker;

//    tracker.setUntrackThreshold(104);
//    EXPECT_EQ(tracker.getUntrackThreshold(), 104);
//    tracker.setUntrackThreshold(-123);
//    EXPECT_EQ(tracker.getUntrackThreshold(), 104);
//}


//// test tracker's track member function
//TEST(TestTrack, MainFunc) {

//    IouTracker tracker;
//    TrackerPersonInfo *person;
//    std::shared_ptr<TrackerFace> face;
//    tracker.setFaceListMaxLen(10);
//    cv::Mat example(200, 200, CV_8UC3, cv::Scalar(0, 255, 255));

//    /*
//     * firstly, tracker analyzes face box
//     */
//    // assume input args of track
//    std::shared_ptr<Frame> frame = std::make_shared<Frame>();
//    cv::cuda::setDevice(0);
//    frame->mOrigImage = std::make_shared<Image>(example.rows, example.cols, example.channels(),  GPU_DEVICE(0));
//    cv::cuda::GpuMat gpuMat(example.rows, example.cols, CV_8UC3, frame->mOrigImage->mData);
//    gpuMat.upload(example);
//    frame->mPayload = std::make_shared<WorkerPayload>();
//    frame->mPayload->mTaskId = "1";
//    std::vector<Face> &faceBox_vec = frame->mFaces;
//    Face faceBox;
//    faceBox.mFaceBox = cv::Rect(cv::Point(10, 10), cv::Point(100, 100));
//    faceBox.mLandmarks.push_back(cv::Point(1, 1));
//    faceBox.mLandmarks.push_back(cv::Point(1, 1));
//    faceBox.mLandmarks.push_back(cv::Point(50, 50));
//    faceBox.mCropBox = cv::Rect(cv::Point(5, 5), cv::Point(105, 105));
//    faceBox_vec.push_back(faceBox);
//    std::string frame1_id("101");

//    tracker.track(frame);

////    EXPECT_EQ(tracker.getPersonInfoList().size(), 1);
////    // a person should be created
////    person = tracker.getPersonInfoList().front();
////    EXPECT_EQ(person->untrackThreshold, tracker.getUntrackThreshold());
////    EXPECT_STRNE(person->id.c_str(), NULL);

////    face = person->faceList.front();
////    // only one face, so it's the latest face
////    EXPECT_EQ(person->latestFace, face);
//    //EXPECT_STREQ(face->face_id.c_str(), "1");
//    //EXPECT_EQ(face->mFaceBox.x, 10);


//    /*
//     * secondly, new face should be list into the old person
//     */
//    faceBox_vec.clear();
//    faceBox.mFaceBox = cv::Rect(cv::Point(20, 20), cv::Point(110, 110));
//    faceBox_vec.push_back(faceBox);
//    std::string frame2_id("102");

//    tracker.track(frame);

//    // new Face.has enough overlapped area with older face,
//    // so new face should be list into the old person
////    EXPECT_EQ(tracker.getPersonInfoList().size(), 1);


//    /*
//     * thirdly, new face should be list into a new person
//     */
//    faceBox_vec.clear();
//    faceBox.mFaceBox = cv::Rect(cv::Point(100, 100), cv::Point(200, 200));
//    faceBox_vec.push_back(faceBox);
//    std::string frame3_id("103");

//    tracker.track(frame);

//    // this Face.has not enough overlapped area with older face,
////    // so create a new person and list the new face in it.
////    EXPECT_EQ(tracker.getPersonInfoList().size(), 2);
////    person = tracker.getPersonInfoList().front();
////    EXPECT_EQ(person->faceList.size(), 2);
////    person = tracker.getPersonInfoList().back();
////    EXPECT_EQ(person->faceList.size(), 1);


//    /*
//     * fourthly, create lots of faces for same person,
//     * and only 10 latest faces is list
//     */
//    for (int i = 0; i < 10000; i++) {
//        std::stringstream stream;
//        stream << i;
//        faceBox_vec.clear();
//        faceBox.mFaceBox = cv::Rect(cv::Point(100, 100), cv::Point(200, 200));
//        faceBox_vec.push_back(faceBox);
//        tracker.track(frame);
//    }

//    // the older person has no tracked face and is removed
////    EXPECT_EQ(tracker.getPersonInfoList().size(), 1);
//    // the person should have 10 latest faces in list
//    //EXPECT_EQ(person->faceList.size(), 10);
//    //face = person->faceList.front();
//    //EXPECT_STREQ(face->face_id.c_str(), "9990");

//    /*
//     * fifthly, best face should be empty, since the dummy image
//     * doesn't include a real face
//     * adjust sharpness and frontRatio to select a best face
//     */
//    //EXPECT_EQ(basic_faces.facesData.size(), 0);
//    tracker.setSharpnessVariance(0);
//    tracker.setFrontRatio(1000);
//    faceBox.mFaceBox = cv::Rect(cv::Point(100, 100), cv::Point(200, 200));
//    faceBox_vec.push_back(faceBox);
//    tracker.track(frame);
//    //EXPECT_EQ(basic_faces.facesData.size(), 1);
//}
