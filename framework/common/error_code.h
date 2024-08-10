//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_ERROR_CODE_H_
#define SOPHON_STREAM_COMMON_ERROR_CODE_H_
#include <string>
#include <unordered_map>
namespace sophon_stream {
namespace common {

enum class ErrorCode {
  SUCCESS = 0,
  UNKNOWN = 1,
  TIMEOUT = 2,
  NO_SUCH_WORKER = 3,
  NO_SUCH_WORKER_ID = 4,
  NO_SUCH_WORKER_PORT = 5,
  NO_SUCH_GRAPH_ID = 6,
  PARSE_CONFIGURE_FAIL = 7,
  THREAD_STATUS_ERROR = 8,
  REPEATED_WORKER_NAME = 9,
  REPEATED_WORKER_ID = 10,
  REPEATED_GRAPH_ID = 11,
  DLOPEN_FAIL = 12,
  MAKE_ALGORITHM_API_FAIL = 13,
  PARAMETER_ERROR = 14,
  DECODE_CHANNEL_USED = 15,
  DECODE_CHANNEL_NOT_FOUND = 16,
  MKDIR_FAIL = 17,
  DECODE_PICTURE_FAILED = 18,
  NO_REGISTER_ALGORITHM = 19,
  ALGORITHM_FACTORY_MAKE_FAIL = 20,
  ERR_METADATA_SIZE = 21,
  DATA_PIPE_FULL = 22,
  DECODE_CHANNEL_PIPE_FULL = 23,

  ERR_FFMPEG_FIND_ENCODER = 1000,      // Can not find encoder
  ERR_FFMPEG_AVCODEC_CTX_ALLOC,        // avcodec context alloc failed
  ERR_FFMPEG_OPEN_CODEC,               // Failed to open decoder
  ERR_FFMPEG_ALLOC_AVFRAME,            // Failed to alloc avframe
  ERR_FFMPEG_NONE_HWDEVICE,            // Failed to find HW device
  ERR_FFMPEG_DEC_NOTSUP_HWDEV,         // Decoder does not support HW device
  ERR_FFMPEG_NONE_HW_DEC,              // Failed to find hw decoder
  ERR_FFMPEG_CREATE_HW_DEV,            // Failed to create specified HW device
  ERR_FFMPEG_PACKET_ALLOC,             // Error alloc packet
  ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM,    // alloc frame outof memory
  ERR_DECODER_EBUSY,                   // all decode channels are busy
  ERR_FFMPEG_OUTPUT_CTX_ALLOC = 3000,  // output format context alloc failed
  ERR_FFMPEG_OUTPUT_URL_OPEN,          // Could not open output URL
  ERR_FFMPEG_OUTSTREAM_ALLOC,          // out stream avformat_new_stream failed
  ERR_FFMPEG_INPUT_CTX_OPEN,           // input format context open failed
  ERR_FFMPEG_FIND_STREAM,              // Can not find input stream information
  ERR_FFMPEG_FIND_VIDEO_STREAM,   // Can not find input video stream information
  ERR_FFMPEG_READ_FRAME,          // Failed to read frame
  ERR_FFMPEG_DURING_DECODING,     // Error during decoding
  ERR_FFMPEG_WHILE_DECODING,      // Error while decoding
  ERR_FFMPEG_SEND_FRAME_ENCODE,   // Error sending a frame for encoding
  ERR_FFMPEG_RECIEV_PKT_ENCODE,   // Error receiveing a pkt for encoding
  ERR_FFMPEG_WRITE_OUTPUT_HEAD,   // Failed to write output head
  ERR_FFMPEG_PARAM_COPY,          // Failed to copy parameters
  ERR_FFMPEG_SET_HWFRAME_CTX,     // Failed to set hwframe context
  ERR_FFMPEG_HWFRAME_GETBUFFER,   // Failed to get hwframe buffer
  ERR_FFMPEG_HWFRAME_CTXNULL,     // HW context is null
  ERR_FFMPEG_SWS_CONTEXT_GET,     // Failed to get swscale context
  ERR_FFMPEG_SWS_SCALE,           // Failed to scale
  ERR_FFMPEG_INPUT_CTX_ALLOC,     // input format context alloc failed
  ERR_FFMPEG_WRITE_FRAME,         // Failed to write frame
  ERR_DECODER_PAYLOAD_JSON,       // payload json analys failed
  ERR_WORKER_DECODER_NULL_IMAGE,  // get null image
  ERR_CUDA_MEMCOPY,               // Failed to copy cuda memory
  ERR_PAYLOAD_NULL,               // payload is nullptr
  ERR_PAYLOAD_DATA_NULL,          // payload data is nullptr
  ERR_TASKID_EXIST,               // taskid existed
  STREAM_END,                     // end of stream
  NOT_VIDEO_CHANNEL,              // not video channel
  ERR_FFMPEG_CODEC_LENGHT,        // format width or height is 0
  ERR_NEW = 5000,
  ERR_MODEL_PATH = 5001,
  ERR_DEVDESC_ALGORITHM = 5002,
  ERR_TRT_RUNTIME = 5003,
  ERR_TRT_ENGINE = 5004,
  ERR_TRT_CONTEXT = 5005,
  ERR_TRT_INFERENCE = 5006,
  ERR_STREAM_INVALID_DEVICE = 5007,
  ERR_STREAM_INVALID_VALUE = 5008,
  ERR_STREAM_MEMORY_ALLOCATION = 5009,
  ERR_STREAM_INVALID_DEVICE_POINTER = 5010,
  ERR_STREAM_INITIALIZATION = 5011,
  ERR_STREAM_INVALID_MEMCPY_DIRECTION = 5012,
  ERR_STREAM_INVALID_RESOURCE_HANDLE = 5013,
  ERR_STREAM_DEVICE_IN_USE = 5014,
  ERR_MXNET_CONTEXT = 5015,
  ERR_MXNET_INPUT_DATA = 5016,
  ERR_MXNET_INFERENCE = 5017,
  ERR_MXNET_OUTPUT_DATA = 5018,
  ERR_WEIGHT_PATH = 5019,
  ERR_SESSION_CREATE = 5020,
  ERR_TENSORFLOW_INFERENCE = 5021,
  ERR_OVER_BOUNDARY = 5022,
  ERR_IMAGE_DATA = 5023,
  ERR_IMAGE_SIZE = 5024,
  ERR_IMAGE_DEVDESC = 5025,
  ERR_IMAGE_TYPE = 5026,
  ERR_IMAGE_ORDER = 5027,
  ERR_CUDA_SET_DEV = 5028,
  ERR_WARPAFFINE = 5029,
  ERR_OPENCV_CONVERT = 5030,
  ERR_CUDA_STREAM_CREATE = 5031,
  ERR_TENSORFLOW_ALLOCATE = 5032,
  ERR_PNET_EXCEPTION = 5033,
  ERR_RNET_EXCEPTION = 5034,
  ERR_ONET_EXCEPTION = 5035,
  ERR_CUDA_DEVICE_NOCOMPATIBLE = 5036,
  ERR_FACE_ALIGN_OUTPU_SIZE = 5037,
  ERR_NODE_SHAPE_WITH_TRT_ENGINE = 5038,
  ERR_LYN_DEVICE_INIT = 5039,
  ERR_LYN_DEVICE_COUNT = 5040,
  ERR_LYN_DEVICE_CONTEXT = 5040,
  ERR_LYN_MODEL_MANAGER = 5041,
  ERR_LYN_INFERENCE = 5042,
  ERR_LYN_NEW_IO = 5043,
  ERR_LYN_DESTROY = 5044,
  ERR_CUDA_FAIL = 5045,
  ERR_OPENCV_UPLOAD = 5046,
  ERR_OPENCV_DOWNLOAD = 5047,
  ERR_OPENCV_CROP = 5048,
};
const std::unordered_map<ErrorCode, std::string> ErrorCodeMap = {
    {ErrorCode::SUCCESS, "SUCCESS"},
    {ErrorCode::UNKNOWN, "UNKNOWN"},
    {ErrorCode::TIMEOUT, "TIMEOUT"},
    {ErrorCode::NO_SUCH_WORKER, "NO_SUCH_WORKER"},
    {ErrorCode::NO_SUCH_WORKER_ID, "NO_SUCH_WORKER_ID"},
    {ErrorCode::NO_SUCH_WORKER_PORT, "NO_SUCH_WORKER_PORT"},
    {ErrorCode::NO_SUCH_GRAPH_ID, "NO_SUCH_GRAPH_ID"},
    {ErrorCode::PARSE_CONFIGURE_FAIL, "PARSE_CONFIGURE_FAIL"},
    {ErrorCode::THREAD_STATUS_ERROR, "THREAD_STATUS_ERROR"},
    {ErrorCode::REPEATED_WORKER_NAME, "REPEATED_WORKER_NAME"},
    {ErrorCode::REPEATED_WORKER_ID, "REPEATED_WORKER_ID"},
    {ErrorCode::REPEATED_GRAPH_ID, "REPEATED_GRAPH_ID"},
    {ErrorCode::DLOPEN_FAIL, "DLOPEN_FAIL"},
    {ErrorCode::MAKE_ALGORITHM_API_FAIL, "MAKE_ALGORITHM_API_FAIL"},
    {ErrorCode::PARAMETER_ERROR, "PARAMETER_ERROR"},
    {ErrorCode::DECODE_CHANNEL_USED, "DECODE_CHANNEL_USED"},
    {ErrorCode::DECODE_CHANNEL_NOT_FOUND, "DECODE_CHANNEL_NOT_FOUND"},
    {ErrorCode::MKDIR_FAIL, "MKDIR_FAIL"},
    {ErrorCode::DECODE_PICTURE_FAILED, "DECODE_PICTURE_FAILED"},
    {ErrorCode::NO_REGISTER_ALGORITHM, "NO_REGISTER_ALGORITHM"},
    {ErrorCode::ALGORITHM_FACTORY_MAKE_FAIL, "ALGORITHM_FACTORY_MAKE_FAIL"},
    {ErrorCode::ERR_METADATA_SIZE, "ERR_METADATA_SIZE"},
    {ErrorCode::DATA_PIPE_FULL, "DATA_PIPE_FULL"},
    {ErrorCode::DECODE_CHANNEL_PIPE_FULL, "DECODE_CHANNEL_PIPE_FULL"},
    {ErrorCode::ERR_FFMPEG_FIND_ENCODER, "ERR_FFMPEG_FIND_ENCODER"},
    {ErrorCode::ERR_FFMPEG_AVCODEC_CTX_ALLOC, "ERR_FFMPEG_AVCODEC_CTX_ALLOC"},
    {ErrorCode::ERR_FFMPEG_OPEN_CODEC, "ERR_FFMPEG_OPEN_CODEC"},
    {ErrorCode::ERR_FFMPEG_ALLOC_AVFRAME, "ERR_FFMPEG_ALLOC_AVFRAME"},
    {ErrorCode::ERR_FFMPEG_NONE_HWDEVICE, "ERR_FFMPEG_NONE_HWDEVICE"},
    {ErrorCode::ERR_FFMPEG_DEC_NOTSUP_HWDEV, "ERR_FFMPEG_DEC_NOTSUP_HWDEV"},
    {ErrorCode::ERR_FFMPEG_NONE_HW_DEC, "ERR_FFMPEG_NONE_HW_DEC"},
    {ErrorCode::ERR_FFMPEG_CREATE_HW_DEV, "ERR_FFMPEG_CREATE_HW_DEV"},
    {ErrorCode::ERR_FFMPEG_PACKET_ALLOC, "ERR_FFMPEG_PACKET_ALLOC"},
    {ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM, "ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM"},
    {ErrorCode::ERR_DECODER_EBUSY, "ERR_DECODER_EBUSY"},
    {ErrorCode::ERR_FFMPEG_OUTPUT_CTX_ALLOC, "ERR_FFMPEG_OUTPUT_CTX_ALLOC"},
    {ErrorCode::ERR_FFMPEG_OUTPUT_URL_OPEN, "ERR_FFMPEG_OUTPUT_URL_OPEN"},
    {ErrorCode::ERR_FFMPEG_OUTSTREAM_ALLOC, "ERR_FFMPEG_OUTSTREAM_ALLOC"},
    {ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN, "ERR_FFMPEG_INPUT_CTX_OPEN"},
    {ErrorCode::ERR_FFMPEG_FIND_STREAM, "ERR_FFMPEG_FIND_STREAM"},
    {ErrorCode::ERR_FFMPEG_FIND_VIDEO_STREAM, "ERR_FFMPEG_FIND_VIDEO_STREAM"},
    {ErrorCode::ERR_FFMPEG_READ_FRAME, "ERR_FFMPEG_READ_FRAME"},
    {ErrorCode::ERR_FFMPEG_DURING_DECODING, "ERR_FFMPEG_DURING_DECODING"},
    {ErrorCode::ERR_FFMPEG_WHILE_DECODING, "ERR_FFMPEG_WHILE_DECODING"},
    {ErrorCode::ERR_FFMPEG_SEND_FRAME_ENCODE, "ERR_FFMPEG_SEND_FRAME_ENCODE"},
    {ErrorCode::ERR_FFMPEG_RECIEV_PKT_ENCODE, "ERR_FFMPEG_RECIEV_PKT_ENCODE"},
    {ErrorCode::ERR_FFMPEG_WRITE_OUTPUT_HEAD, "ERR_FFMPEG_WRITE_OUTPUT_HEAD"},
    {ErrorCode::ERR_FFMPEG_PARAM_COPY, "ERR_FFMPEG_PARAM_COPY"},
    {ErrorCode::ERR_FFMPEG_SET_HWFRAME_CTX, "ERR_FFMPEG_SET_HWFRAME_CTX"},
    {ErrorCode::ERR_FFMPEG_HWFRAME_GETBUFFER, "ERR_FFMPEG_HWFRAME_GETBUFFER"},
    {ErrorCode::ERR_FFMPEG_HWFRAME_CTXNULL, "ERR_FFMPEG_HWFRAME_CTXNULL"},
    {ErrorCode::ERR_FFMPEG_SWS_CONTEXT_GET, "ERR_FFMPEG_SWS_CONTEXT_GET"},
    {ErrorCode::ERR_FFMPEG_SWS_SCALE, "ERR_FFMPEG_SWS_SCALE"},
    {ErrorCode::ERR_FFMPEG_INPUT_CTX_ALLOC, "ERR_FFMPEG_INPUT_CTX_ALLOC"},
    {ErrorCode::ERR_FFMPEG_WRITE_FRAME, "ERR_FFMPEG_WRITE_FRAME"},
    {ErrorCode::ERR_DECODER_PAYLOAD_JSON, "ERR_DECODER_PAYLOAD_JSON"},
    {ErrorCode::ERR_WORKER_DECODER_NULL_IMAGE, "ERR_WORKER_DECODER_NULL_IMAGE"},
    {ErrorCode::ERR_CUDA_MEMCOPY, "ERR_CUDA_MEMCOPY"},
    {ErrorCode::ERR_PAYLOAD_NULL, "ERR_PAYLOAD_NULL"},
    {ErrorCode::ERR_PAYLOAD_DATA_NULL, "ERR_PAYLOAD_DATA_NULL"},
    {ErrorCode::ERR_TASKID_EXIST, "ERR_TASKID_EXIST"},
    {ErrorCode::STREAM_END, "STREAM_END"},
    {ErrorCode::NOT_VIDEO_CHANNEL, "NOT_VIDEO_CHANNEL"},
    {ErrorCode::ERR_FFMPEG_CODEC_LENGHT, "ERR_FFMPEG_CODEC_LENGHT"},
    {ErrorCode::ERR_NEW, "ERR_NEW"},
    {ErrorCode::ERR_MODEL_PATH, "ERR_MODEL_PATH"},
    {ErrorCode::ERR_DEVDESC_ALGORITHM, "ERR_DEVDESC_ALGORITHM"},
    {ErrorCode::ERR_TRT_RUNTIME, "ERR_TRT_RUNTIME"},
    {ErrorCode::ERR_TRT_ENGINE, "ERR_TRT_ENGINE"},
    {ErrorCode::ERR_TRT_CONTEXT, "ERR_TRT_CONTEXT"},
    {ErrorCode::ERR_TRT_INFERENCE, "ERR_TRT_INFERENCE"},
    {ErrorCode::ERR_STREAM_INVALID_DEVICE, "ERR_STREAM_INVALID_DEVICE"},
    {ErrorCode::ERR_STREAM_INVALID_VALUE, "ERR_STREAM_INVALID_VALUE"},
    {ErrorCode::ERR_STREAM_MEMORY_ALLOCATION, "ERR_STREAM_MEMORY_ALLOCATION"},
    {ErrorCode::ERR_STREAM_INVALID_DEVICE_POINTER, "ERR_STREAM_INVALID_DEVICE_POINTER"},
    {ErrorCode::ERR_STREAM_INITIALIZATION, "ERR_STREAM_INITIALIZATION"},
    {ErrorCode::ERR_STREAM_INVALID_MEMCPY_DIRECTION, "ERR_STREAM_INVALID_MEMCPY_DIRECTION"},
    {ErrorCode::ERR_STREAM_INVALID_RESOURCE_HANDLE, "ERR_STREAM_INVALID_RESOURCE_HANDLE"},
    {ErrorCode::ERR_STREAM_DEVICE_IN_USE, "ERR_STREAM_DEVICE_IN_USE"},
    {ErrorCode::ERR_MXNET_CONTEXT, "ERR_MXNET_CONTEXT"},
    {ErrorCode::ERR_MXNET_INPUT_DATA, "ERR_MXNET_INPUT_DATA"},
    {ErrorCode::ERR_MXNET_INFERENCE, "ERR_MXNET_INFERENCE"},
    {ErrorCode::ERR_MXNET_OUTPUT_DATA, "ERR_MXNET_OUTPUT_DATA"},
    {ErrorCode::ERR_WEIGHT_PATH, "ERR_WEIGHT_PATH"},
    {ErrorCode::ERR_SESSION_CREATE, "ERR_SESSION_CREATE"},
    {ErrorCode::ERR_TENSORFLOW_INFERENCE, "ERR_TENSORFLOW_INFERENCE"},
    {ErrorCode::ERR_OVER_BOUNDARY, "ERR_OVER_BOUNDARY"},
    {ErrorCode::ERR_IMAGE_DATA, "ERR_IMAGE_DATA"},
    {ErrorCode::ERR_IMAGE_SIZE, "ERR_IMAGE_SIZE"},
    {ErrorCode::ERR_IMAGE_DEVDESC, "ERR_IMAGE_DEVDESC"},
    {ErrorCode::ERR_IMAGE_TYPE, "ERR_IMAGE_TYPE"},
    {ErrorCode::ERR_IMAGE_ORDER, "ERR_IMAGE_ORDER"},
    {ErrorCode::ERR_CUDA_SET_DEV, "ERR_CUDA_SET_DEV"},
    {ErrorCode::ERR_WARPAFFINE, "ERR_WARPAFFINE"},
    {ErrorCode::ERR_OPENCV_CONVERT, "ERR_OPENCV_CONVERT"},
    {ErrorCode::ERR_CUDA_STREAM_CREATE, "ERR_CUDA_STREAM_CREATE"},
    {ErrorCode::ERR_TENSORFLOW_ALLOCATE, "ERR_TENSORFLOW_ALLOCATE"},
    {ErrorCode::ERR_PNET_EXCEPTION, "ERR_PNET_EXCEPTION"},
    {ErrorCode::ERR_RNET_EXCEPTION, "ERR_RNET_EXCEPTION"},
    {ErrorCode::ERR_ONET_EXCEPTION, "ERR_ONET_EXCEPTION"},
    {ErrorCode::ERR_CUDA_DEVICE_NOCOMPATIBLE, "ERR_CUDA_DEVICE_NOCOMPATIBLE"},
    {ErrorCode::ERR_FACE_ALIGN_OUTPU_SIZE, "ERR_FACE_ALIGN_OUTPU_SIZE"},
    {ErrorCode::ERR_NODE_SHAPE_WITH_TRT_ENGINE, "ERR_NODE_SHAPE_WITH_TRT_ENGINE"},
    {ErrorCode::ERR_LYN_DEVICE_INIT, "ERR_LYN_DEVICE_INIT"},
    {ErrorCode::ERR_LYN_DEVICE_COUNT, "ERR_LYN_DEVICE_COUNT"},
    // {ErrorCode::ERR_LYN_DEVICE_CONTEXT, "ERR_LYN_DEVICE_CONTEXT"}, // This appears to be commented out in your original switch
    {ErrorCode::ERR_LYN_MODEL_MANAGER, "ERR_LYN_MODEL_MANAGER"},
    {ErrorCode::ERR_LYN_INFERENCE, "ERR_LYN_INFERENCE"},
    {ErrorCode::ERR_LYN_NEW_IO, "ERR_LYN_NEW_IO"},
    {ErrorCode::ERR_LYN_DESTROY, "ERR_LYN_DESTROY"},
    {ErrorCode::ERR_CUDA_FAIL, "ERR_CUDA_FAIL"},
    {ErrorCode::ERR_OPENCV_UPLOAD, "ERR_OPENCV_UPLOAD"},
    {ErrorCode::ERR_OPENCV_DOWNLOAD, "ERR_OPENCV_DOWNLOAD"},
    {ErrorCode::ERR_OPENCV_CROP, "ERR_OPENCV_CROP"},
    // Add any additional error codes and their string representations here
};
static std::string ErrorCodeToString(ErrorCode code) {
  switch (code) {
    case ErrorCode::SUCCESS:
      return "SUCCESS";
    case ErrorCode::UNKNOWN:
      return "UNKNOWN";
    case ErrorCode::TIMEOUT:
      return "TIMEOUT";
    case ErrorCode::NO_SUCH_WORKER:
      return "NO_SUCH_WORKER";
    case ErrorCode::NO_SUCH_WORKER_ID:
      return "NO_SUCH_WORKER_ID";
    case ErrorCode::NO_SUCH_WORKER_PORT:
      return "NO_SUCH_WORKER_PORT";
    case ErrorCode::NO_SUCH_GRAPH_ID:
      return "NO_SUCH_GRAPH_ID";
    case ErrorCode::PARSE_CONFIGURE_FAIL:
      return "PARSE_CONFIGURE_FAIL";
    case ErrorCode::THREAD_STATUS_ERROR:
      return "THREAD_STATUS_ERROR";
    case ErrorCode::REPEATED_WORKER_NAME:
      return "REPEATED_WORKER_NAME";
    case ErrorCode::REPEATED_WORKER_ID:
      return "REPEATED_WORKER_ID";
    case ErrorCode::REPEATED_GRAPH_ID:
      return "REPEATED_GRAPH_ID";
    case ErrorCode::DLOPEN_FAIL:
      return "DLOPEN_FAIL";
    case ErrorCode::MAKE_ALGORITHM_API_FAIL:
      return "MAKE_ALGORITHM_API_FAIL";
    case ErrorCode::PARAMETER_ERROR:
      return "PARAMETER_ERROR";
    case ErrorCode::DECODE_CHANNEL_USED:
      return "DECODE_CHANNEL_USED";
    case ErrorCode::DECODE_CHANNEL_NOT_FOUND:
      return "DECODE_CHANNEL_NOT_FOUND";
    case ErrorCode::MKDIR_FAIL:
      return "MKDIR_FAIL";
    case ErrorCode::DECODE_PICTURE_FAILED:
      return "DECODE_PICTURE_FAILED";
    case ErrorCode::NO_REGISTER_ALGORITHM:
      return "NO_REGISTER_ALGORITHM";
    case ErrorCode::ALGORITHM_FACTORY_MAKE_FAIL:
      return "ALGORITHM_FACTORY_MAKE_FAIL";
    case ErrorCode::ERR_METADATA_SIZE:
      return "ERR_METADATA_SIZE";
    case ErrorCode::DATA_PIPE_FULL:
      return "DATA_PIPE_FULL";
    case ErrorCode::DECODE_CHANNEL_PIPE_FULL:
      return "CHANNEL_DATA_PIPE_FULL";
    case ErrorCode::ERR_FFMPEG_FIND_ENCODER:
      return "ERR_FFMPEG_FIND_ENCODER";
    case ErrorCode::ERR_FFMPEG_AVCODEC_CTX_ALLOC:
      return "ERR_FFMPEG_AVCODEC_CTX_ALLOC";
    case ErrorCode::ERR_FFMPEG_OPEN_CODEC:
      return "ERR_FFMPEG_OPEN_CODEC";
    case ErrorCode::ERR_FFMPEG_ALLOC_AVFRAME:
      return "ERR_FFMPEG_ALLOC_AVFRAME";
    case ErrorCode::ERR_FFMPEG_NONE_HWDEVICE:
      return "ERR_FFMPEG_NONE_HWDEVICE";
    case ErrorCode::ERR_FFMPEG_DEC_NOTSUP_HWDEV:
      return "ERR_FFMPEG_DEC_NOTSUP_HWDEV";
    case ErrorCode::ERR_FFMPEG_NONE_HW_DEC:
      return "ERR_FFMPEG_NONE_HW_DEC";
    case ErrorCode::ERR_FFMPEG_CREATE_HW_DEV:
      return "ERR_FFMPEG_CREATE_HW_DEV";
    case ErrorCode::ERR_FFMPEG_PACKET_ALLOC:
      return "ERR_FFMPEG_PACKET_ALLOC";
    case ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM:
      return "ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM";
    case ErrorCode::ERR_DECODER_EBUSY:
      return "ERR_DECODER_EBUSY";
    case ErrorCode::ERR_FFMPEG_OUTPUT_CTX_ALLOC:
      return "ERR_FFMPEG_OUTPUT_CTX_ALLOC";
    case ErrorCode::ERR_FFMPEG_OUTPUT_URL_OPEN:
      return "ERR_FFMPEG_OUTPUT_URL_OPEN";
    case ErrorCode::ERR_FFMPEG_OUTSTREAM_ALLOC:
      return "ERR_FFMPEG_OUTSTREAM_ALLOC";
    case ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN:
      return "ERR_FFMPEG_INPUT_CTX_OPEN";
    case ErrorCode::ERR_FFMPEG_FIND_STREAM:
      return "ERR_FFMPEG_FIND_STREAM";
    case ErrorCode::ERR_FFMPEG_FIND_VIDEO_STREAM:
      return "ERR_FFMPEG_FIND_VIDEO_STREAM";
    case ErrorCode::ERR_FFMPEG_READ_FRAME:
      return "ERR_FFMPEG_READ_FRAME";
    case ErrorCode::ERR_FFMPEG_DURING_DECODING:
      return "ERR_FFMPEG_DURING_DECODING";
    case ErrorCode::ERR_FFMPEG_WHILE_DECODING:
      return "ERR_FFMPEG_WHILE_DECODING";
    case ErrorCode::ERR_FFMPEG_SEND_FRAME_ENCODE:
      return "ERR_FFMPEG_SEND_FRAME_ENCODE";
    case ErrorCode::ERR_FFMPEG_RECIEV_PKT_ENCODE:
      return "ERR_FFMPEG_RECIEV_PKT_ENCODE";
    case ErrorCode::ERR_FFMPEG_WRITE_OUTPUT_HEAD:
      return "ERR_FFMPEG_WRITE_OUTPUT_HEAD";
    case ErrorCode::ERR_FFMPEG_PARAM_COPY:
      return "ERR_FFMPEG_PARAM_COPY";
    case ErrorCode::ERR_FFMPEG_SET_HWFRAME_CTX:
      return "ERR_FFMPEG_SET_HWFRAME_CTX";
    case ErrorCode::ERR_FFMPEG_HWFRAME_GETBUFFER:
      return "ERR_FFMPEG_HWFRAME_GETBUFFER";
    case ErrorCode::ERR_FFMPEG_HWFRAME_CTXNULL:
      return "ERR_FFMPEG_HWFRAME_CTXNULL";
    case ErrorCode::ERR_FFMPEG_SWS_CONTEXT_GET:
      return "ERR_FFMPEG_SWS_CONTEXT_GET";
    case ErrorCode::ERR_FFMPEG_SWS_SCALE:
      return "ERR_FFMPEG_SWS_SCALE";
    case ErrorCode::ERR_FFMPEG_INPUT_CTX_ALLOC:
      return "ERR_FFMPEG_INPUT_CTX_ALLOC";
    case ErrorCode::ERR_FFMPEG_WRITE_FRAME:
      return "ERR_FFMPEG_WRITE_FRAME";
    case ErrorCode::ERR_DECODER_PAYLOAD_JSON:
      return "ERR_DECODER_PAYLOAD_JSON";
    case ErrorCode::ERR_WORKER_DECODER_NULL_IMAGE:
      return "ERR_WORKER_DECODER_NULL_IMAGE";
    case ErrorCode::ERR_CUDA_MEMCOPY:
      return "ERR_CUDA_MEMCOPY";
    case ErrorCode::ERR_PAYLOAD_NULL:
      return "ERR_PAYLOAD_NULL";
    case ErrorCode::ERR_PAYLOAD_DATA_NULL:
      return "ERR_PAYLOAD_DATA_NULL";
    case ErrorCode::ERR_TASKID_EXIST:
      return "ERR_TASKID_EXIST";
    case ErrorCode::STREAM_END:
      return "STREAM_END";
    case ErrorCode::NOT_VIDEO_CHANNEL:
      return "NOT_VIDEO_CHANNEL";
    case ErrorCode::ERR_FFMPEG_CODEC_LENGHT:
      return "ERR_FFMPEG_CODEC_LENGHT";
    case ErrorCode::ERR_NEW:
      return "ERR_NEW";
    case ErrorCode::ERR_MODEL_PATH:
      return "ERR_MODEL_PATH";
    case ErrorCode::ERR_DEVDESC_ALGORITHM:
      return "ERR_DEVDESC_ALGORITHM";
    case ErrorCode::ERR_TRT_RUNTIME:
      return "ERR_TRT_RUNTIME";
    case ErrorCode::ERR_TRT_ENGINE:
      return "ERR_TRT_ENGINE";
    case ErrorCode::ERR_TRT_CONTEXT:
      return "ERR_TRT_CONTEXT";
    case ErrorCode::ERR_TRT_INFERENCE:
      return "ERR_TRT_INFERENCE";
    case ErrorCode::ERR_STREAM_INVALID_DEVICE:
      return "ERR_STREAM_INVALID_DEVICE";
    case ErrorCode::ERR_STREAM_INVALID_VALUE:
      return "ERR_STREAM_INVALID_VALUE";
    case ErrorCode::ERR_STREAM_MEMORY_ALLOCATION:
      return "ERR_STREAM_MEMORY_ALLOCATION";
    case ErrorCode::ERR_STREAM_INVALID_DEVICE_POINTER:
      return "ERR_STREAM_INVALID_DEVICE_POINTER";
    case ErrorCode::ERR_STREAM_INITIALIZATION:
      return "ERR_STREAM_INITIALIZATION";
    case ErrorCode::ERR_STREAM_INVALID_MEMCPY_DIRECTION:
      return "ERR_STREAM_INVALID_MEMCPY_DIRECTION";
    case ErrorCode::ERR_STREAM_INVALID_RESOURCE_HANDLE:
      return "ERR_STREAM_INVALID_RESOURCE_HANDLE";
    case ErrorCode::ERR_STREAM_DEVICE_IN_USE:
      return "ERR_STREAM_DEVICE_IN_USE";
    case ErrorCode::ERR_MXNET_CONTEXT:
      return "ERR_MXNET_CONTEXT";
    case ErrorCode::ERR_MXNET_INPUT_DATA:
      return "ERR_MXNET_INPUT_DATA";
    case ErrorCode::ERR_MXNET_INFERENCE:
      return "ERR_MXNET_INFERENCE";
    case ErrorCode::ERR_MXNET_OUTPUT_DATA:
      return "ERR_MXNET_OUTPUT_DATA";
    case ErrorCode::ERR_WEIGHT_PATH:
      return "ERR_WEIGHT_PATH";
    case ErrorCode::ERR_SESSION_CREATE:
      return "ERR_SESSION_CREATE";
    case ErrorCode::ERR_TENSORFLOW_INFERENCE:
      return "ERR_TENSORFLOW_INFERENCE";
    case ErrorCode::ERR_OVER_BOUNDARY:
      return "ERR_OVER_BOUNDARY";
    case ErrorCode::ERR_IMAGE_DATA:
      return "ERR_IMAGE_DATA";
    case ErrorCode::ERR_IMAGE_SIZE:
      return "ERR_IMAGE_SIZE";
    case ErrorCode::ERR_IMAGE_DEVDESC:
      return "ERR_IMAGE_DEVDESC";
    case ErrorCode::ERR_IMAGE_TYPE:
      return "ERR_IMAGE_TYPE";
    case ErrorCode::ERR_IMAGE_ORDER:
      return "ERR_IMAGE_ORDER";
    case ErrorCode::ERR_CUDA_SET_DEV:
      return "ERR_CUDA_SET_DEV";
    case ErrorCode::ERR_WARPAFFINE:
      return "ERR_WARPAFFINE";
    case ErrorCode::ERR_OPENCV_CONVERT:
      return "ERR_OPENCV_CONVERT";
    case ErrorCode::ERR_CUDA_STREAM_CREATE:
      return "ERR_CUDA_STREAM_CREATE";
    case ErrorCode::ERR_TENSORFLOW_ALLOCATE:
      return "ERR_TENSORFLOW_ALLOCATE";
    case ErrorCode::ERR_PNET_EXCEPTION:
      return "ERR_PNET_EXCEPTION";
    case ErrorCode::ERR_RNET_EXCEPTION:
      return "ERR_RNET_EXCEPTION";
    case ErrorCode::ERR_ONET_EXCEPTION:
      return "ERR_ONET_EXCEPTION";
    case ErrorCode::ERR_CUDA_DEVICE_NOCOMPATIBLE:
      return "ERR_CUDA_DEVICE_NOCOMPATIBLE";
    case ErrorCode::ERR_FACE_ALIGN_OUTPU_SIZE:
      return "ERR_FACE_ALIGN_OUTPU_SIZE";
    case ErrorCode::ERR_NODE_SHAPE_WITH_TRT_ENGINE:
      return "ERR_NODE_SHAPE_WITH_TRT_ENGINE";
    case ErrorCode::ERR_LYN_DEVICE_INIT:
      return "ERR_LYN_DEVICE_INIT";
    case ErrorCode::ERR_LYN_DEVICE_COUNT:
      return "ERR_LYN_DEVICE_COUNT";
    // case ErrorCode::ERR_LYN_DEVICE_CONTEXT: return "ERR_LYN_DEVICE_CONTEXT";
    case ErrorCode::ERR_LYN_MODEL_MANAGER:
      return "ERR_LYN_MODEL_MANAGER";
    case ErrorCode::ERR_LYN_INFERENCE:
      return "ERR_LYN_INFERENCE";
    case ErrorCode::ERR_LYN_NEW_IO:
      return "ERR_LYN_NEW_IO";
    case ErrorCode::ERR_LYN_DESTROY:
      return "ERR_LYN_DESTROY";
    case ErrorCode::ERR_CUDA_FAIL:
      return "ERR_CUDA_FAIL";
    case ErrorCode::ERR_OPENCV_UPLOAD:
      return "ERR_OPENCV_UPLOAD";
    case ErrorCode::ERR_OPENCV_DOWNLOAD:
      return "ERR_OPENCV_DOWNLOAD";
    case ErrorCode::ERR_OPENCV_CROP:
      return "ERR_OPENCV_CROP";
    default:
      return "Unknown Error";
  }
}
}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_ERROR_CODE_H_