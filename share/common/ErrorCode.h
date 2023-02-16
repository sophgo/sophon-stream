#pragma once

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
    DECODE_CHANNEL_USED   = 15,
    DECODE_CHANNEL_NOT_FOUND = 16,
    MKDIR_FAIL = 17,
    DECODE_PICTURE_FAILED = 18, 
    NO_REGISTER_ALGORITHM = 19,
    ALGORITHM_FACTORY_MAKE_FAIL = 20,
    ERR_METADATA_SIZE = 21, 


    ERR_FFMPEG_FIND_ENCODER            = 1000, // Can not find encoder
    ERR_FFMPEG_AVCODEC_CTX_ALLOC            ,  // avcodec context alloc failed
    ERR_FFMPEG_OPEN_CODEC                   ,  // Failed to open decoder
    ERR_FFMPEG_ALLOC_AVFRAME                ,  // Failed to alloc avframe
    ERR_FFMPEG_NONE_HWDEVICE                ,  // Failed to find HW device
    ERR_FFMPEG_DEC_NOTSUP_HWDEV             ,  // Decoder does not support HW device
    ERR_FFMPEG_NONE_HW_DEC                  ,  // Failed to find hw decoder
    ERR_FFMPEG_CREATE_HW_DEV                ,  // Failed to create specified HW device
    ERR_FFMPEG_PACKET_ALLOC                 ,  // Error alloc packet
    ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM        ,  // alloc frame outof memory
    ERR_DECODER_EBUSY                       ,  // all decode channels are busy
    ERR_FFMPEG_OUTPUT_CTX_ALLOC       = 3000,  // output format context alloc failed
    ERR_FFMPEG_OUTPUT_URL_OPEN              ,  // Could not open output URL
    ERR_FFMPEG_OUTSTREAM_ALLOC              ,  // out stream avformat_new_stream failed
    ERR_FFMPEG_INPUT_CTX_OPEN               ,  // input format context open failed
    ERR_FFMPEG_FIND_STREAM                  ,  // Can not find input stream information
    ERR_FFMPEG_FIND_VIDEO_STREAM            ,  // Can not find input video stream information
    ERR_FFMPEG_READ_FRAME                   ,  // Failed to read frame
    ERR_FFMPEG_DURING_DECODING              ,  // Error during decoding
    ERR_FFMPEG_WHILE_DECODING               ,  // Error while decoding
    ERR_FFMPEG_SEND_FRAME_ENCODE            ,  // Error sending a frame for encoding
    ERR_FFMPEG_RECIEV_PKT_ENCODE            ,  // Error receiveing a pkt for encoding
    ERR_FFMPEG_WRITE_OUTPUT_HEAD            ,  // Failed to write output head
    ERR_FFMPEG_PARAM_COPY                   ,  // Failed to copy parameters
    ERR_FFMPEG_SET_HWFRAME_CTX              ,  // Failed to set hwframe context
    ERR_FFMPEG_HWFRAME_GETBUFFER            ,  // Failed to get hwframe buffer
    ERR_FFMPEG_HWFRAME_CTXNULL              ,  // HW context is null
    ERR_FFMPEG_SWS_CONTEXT_GET              ,  // Failed to get swscale context
    ERR_FFMPEG_SWS_SCALE                    ,  // Failed to scale
    ERR_FFMPEG_INPUT_CTX_ALLOC              ,  // input format context alloc failed
    ERR_FFMPEG_WRITE_FRAME                  ,  // Failed to write frame
    ERR_DECODER_PAYLOAD_JSON                ,  // payload json analys failed
    ERR_WORKER_DECODER_NULL_IMAGE           ,  // get null image
    ERR_CUDA_MEMCOPY                        ,  // Failed to copy cuda memory
    ERR_PAYLOAD_NULL                        ,  // payload is nullptr
    ERR_PAYLOAD_DATA_NULL                   ,  // payload data is nullptr
    ERR_TASKID_EXIST                        ,  // taskid existed
    STREAM_END                              ,  // end of stream
    NOT_VIDEO_CHANNEL                       ,  // not video channel
    ERR_FFMPEG_CODEC_LENGHT                 ,  // format width or height is 0
    ERR_NEW = 5000,
    ERR_MODEL_PATH = 5001,
    ERR_DEVDESC_ALGORITHM = 5002,
    ERR_TRT_RUNTIME = 5003,
    ERR_TRT_ENGINE = 5004,
    ERR_TRT_CONTEXT = 5005,
    ERR_TRT_INFERENCE = 5006,
    ERR_CUDA_INVALID_DEVICE = 5007,
    ERR_CUDA_INVALID_VALUE = 5008,
    ERR_CUDA_MEMORY_ALLOCATION = 5009,
    ERR_CUDA_INVALID_DEVICE_POINTER = 5010,
    ERR_CUDA_INITIALIZATION = 5011,
    ERR_CUDA_INVALID_MEMCPY_DIRECTION = 5012,
    ERR_CUDA_INVALID_RESOURCE_HANDLE = 5013,
    ERR_CUDA_DEVICE_IN_USE = 5014,
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

} // namespace common
} // namespace sophon_stream

