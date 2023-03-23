#pragma once

#include "./Frame.h"
#include "bmcv_api_ext.h"


namespace sophon_stream {
namespace common {

static bm_image_format_ext format_stream2bmcv(FormatType & t)
{
    bm_image_format_ext format_bmcv;
    switch(t)
    {
        case FormatType::NV12:
            format_bmcv = FORMAT_NV12;
            break;
        case FormatType::NV21:
            format_bmcv = FORMAT_NV21;
            break;
        case FormatType::BGR_PACKET:
            format_bmcv = FORMAT_BGR_PACKED;
            break;
        case FormatType::BGR_PLANAR:
            format_bmcv = FORMAT_BGR_PLANAR;
            break;
        case FormatType::RGB_PACKET:
            format_bmcv = FORMAT_RGB_PACKED;
            break;
        case FormatType::RGB_PLANAR:
            format_bmcv = FORMAT_RGB_PLANAR;
            break;
        default:
            throw "Unsupported type!";
            break;
    }
    return format_bmcv;
}

static FormatType format_bmcv2stream(bm_image_format_ext & t)
{
    FormatType format_stream;
    switch(t)
    {
        case FORMAT_NV12:
            format_stream = FormatType::NV12;
            break;
        case FORMAT_NV21:
            format_stream = FormatType::NV21;
            break;
        case FORMAT_BGR_PACKED:
            format_stream = FormatType::BGR_PACKET;
            break;
        case FORMAT_BGR_PLANAR:
            format_stream = FormatType::BGR_PLANAR;
            break;
        case FORMAT_RGB_PACKED:
            format_stream = FormatType::RGB_PACKET;
            break;
        case FORMAT_RGB_PLANAR:
            format_stream = FormatType::RGB_PLANAR;
            break;
        default:
            format_stream = FormatType::UNKNOWN;
            break;
    }
    return format_stream;
}

static bm_image_data_format_ext data_stream2bmcv(DataType & t)
{
    bm_image_data_format_ext data_bmcv;
    switch(t)
    {
        case DataType::INTEGER:
            data_bmcv = DATA_TYPE_EXT_1N_BYTE;
            break;
        case DataType::FLOATING_POINT:
            data_bmcv = DATA_TYPE_EXT_FLOAT32;
            break;
        default:
            throw "Unsupported type!";
            break;
    }
    return data_bmcv;
}

static DataType data_bmcv2stream(bm_image_data_format_ext & t)
{
    DataType data_stream;
    switch(t)
    {
        case DATA_TYPE_EXT_1N_BYTE:
            data_stream = DataType::INTEGER;
            break;
        case DATA_TYPE_EXT_FLOAT32:
            data_stream = DataType::FLOATING_POINT;
            break;
        default:
            throw "Unsupported type!";
            break;
    }
    return data_stream;
}


}
}