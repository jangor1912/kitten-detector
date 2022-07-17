//
// Created by jan on 02.07.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "decoders.h"
#include "utils/utils.h"

#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

// Functional interface of decoder bin creation - strategy pattern to make video-codec transparent
typedef GstElement *(*CreateDecoderBinFunc)(gchar* bin_name);

GstElement *create_h264_decoder_bin(gchar* bin_name);
GstElement *create_h265_decoder_bin(gchar* bin_name);
void add_nvidia_caps_to_caps_filter(GstElement *caps_filter);

enum VideoCodec codec_from_string(gchar *video_codec_str){
    if(video_codec_str == NULL){
        return UNSUPPORTED;
    }

    if(strcmp(video_codec_str, H264_CODEC_NAME) == 0){
        return H264;
    } else if(strcmp(video_codec_str, H265_CODEC_NAME) == 0){
        return H265;
    }
    return UNSUPPORTED;
}

GstElement *create_decoder_bin(
        guint source_number,
        enum VideoCodec video_codec
){
    GstElement *bin = NULL;
    CreateDecoderBinFunc bin_creation_function;
    gchar bin_name[BIN_NAME_LENGTH];
    gchar *codec_string;
    switch(video_codec){
        case H264:
            bin_creation_function = create_h264_decoder_bin;
            codec_string = H264_CODEC_NAME;
            break;
        case H265:
            bin_creation_function = create_h265_decoder_bin;
            codec_string = H265_CODEC_NAME;
            break;
        default:
            g_printerr("Unknown video-codec - unable to decode stream. Exiting!\n");
            return NULL;
    }
    g_snprintf(
            bin_name, BIN_NAME_LENGTH,
            "%s-decoder-bin-%d",
            codec_string, source_number
            );
    bin = bin_creation_function(bin_name);
    return bin;
}

GstElement *create_h264_decoder_bin(gchar* bin_name){
    GstElement *bin = gst_bin_new(bin_name);

    GstElement *queue = gst_element_factory_make("queue", "pre-decoder-queue");
    GstElement *depay = gst_element_factory_make("rtph264depay", "tps-h264-depay");
    GstElement *decoder = gst_element_factory_make("nvv4l2decoder", "h264-video-decoder");
    GstElement *converter = gst_element_factory_make("nvvideoconvert", "h264-video-converter");
    GstElement *caps_filter = gst_element_factory_make("capsfilter", "h264-video-caps-filter");
    
    if(!bin || !queue || !depay || !decoder || !converter || !caps_filter){
        g_printerr("Unable to create elements of h264 decoder (%s). Exiting!\n", bin_name);
        return NULL;
    }

    add_nvidia_caps_to_caps_filter(caps_filter);

    gst_bin_add_many(GST_BIN(bin), queue, depay, decoder, converter, caps_filter, NULL);

    if(gst_element_link_many(queue, depay, decoder, converter, caps_filter, NULL)){
        g_printerr("Unable to link elements of decoder %s. Exiting!\n", bin_name);
        return NULL;
    }

    add_ghost_sink_pad_to_bin(bin, queue, "sink");
    add_ghost_src_pad_to_bin(bin, caps_filter, "src");

    return bin;
}

GstElement *create_h265_decoder_bin(gchar* bin_name){
    GstElement *bin = gst_bin_new(bin_name);

    GstElement *queue = gst_element_factory_make("queue", "pre-decoder-queue");
    GstElement *depay = gst_element_factory_make("rtph265depay", "tps-h265-depay");
    GstElement *decoder = gst_element_factory_make("nvv4l2decoder", "h265-video-decoder");
    GstElement *converter = gst_element_factory_make("nvvideoconvert", "h265-video-converter");
    GstElement *caps_filter = gst_element_factory_make("capsfilter", "h265-video-caps-filter");

    if(!bin || !queue || !depay || !decoder || !converter || !caps_filter){
        g_printerr("Unable to create elements of h265 decoder (%s). Exiting!\n", bin_name);
        return NULL;
    }

    add_nvidia_caps_to_caps_filter(caps_filter);

    gst_bin_add_many(GST_BIN(bin), queue, depay, decoder, converter, caps_filter, NULL);

    if(gst_element_link_many(queue, depay, decoder, converter, caps_filter, NULL)){
        g_printerr("Unable to link elements of decoder %s. Exiting!\n", bin_name);
        return NULL;
    }

    add_ghost_sink_pad_to_bin(bin, queue, "sink");
    add_ghost_src_pad_to_bin(bin, caps_filter, "src");

    return bin;
}


void add_nvidia_caps_to_caps_filter(GstElement *caps_filter){
    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw", "format",
        G_TYPE_STRING, "NV12", NULL
    );
    GstCapsFeatures *features = gst_caps_features_new(GST_CAPS_FEATURES_NVMM, NULL);
    gst_caps_set_features(caps, 0, features);
    g_object_set(G_OBJECT(caps_filter),
        "caps", caps,
        NULL
    );
    gst_caps_unref(caps);
}
