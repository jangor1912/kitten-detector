//
// Created by jan on 02.07.22.
//

#ifndef KITTEN_DETECTOR_DECODERS_H
#define KITTEN_DETECTOR_DECODERS_H

#include <gst/gst.h>
#include <glib.h>

#define H264_CODEC_NAME "h264"
#define H265_CODEC_NAME "h265"
#define CODEC_NAME_LENGTH 64

enum VideoCodec {
    H264,
    H265,
    UNSUPPORTED
};

enum VideoCodec codec_from_string(gchar *video_codec_str);
GstElement *create_decoder_bin(
        guint source_number,
        enum VideoCodec video_codec
);


#endif //KITTEN_DETECTOR_DECODERS_H
