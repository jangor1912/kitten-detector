//
// Created by jan on 07.11.22.
//

#include "probes/timestamps.h"
#include "probes/probes.h"
#include <sys/time.h>

void set_timestamp(struct timespec *timestamp){
    int result = clock_gettime(CLOCK_MONOTONIC_RAW, timestamp);
    if(result != 0){
        g_printerr("Cannot set timestamp!\n");
    }
}

void add_pre_decoder_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->pre_decoder_timestamp);
}

void add_post_decoder_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->post_decoder_timestamp);
}

void add_pre_inference_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->pre_inference_timestamp);
}

void add_post_inference_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->post_inference_timestamp);
}

void add_pre_tracker_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->pre_tracker_timestamp);
}

void add_post_tracker_timestamp(CommonMetaData *common_meta, gpointer u_data){
    set_timestamp(common_meta->post_tracker_timestamp);
}


GstPadProbeReturn add_pre_decoder_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_pre_decoder_timestamp);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn add_post_decoder_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_post_decoder_timestamp);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn add_pre_inference_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_pre_inference_timestamp);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn add_post_inference_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_post_inference_timestamp);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn add_pre_tracker_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_pre_tracker_timestamp);
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn add_post_tracker_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    on_new_buffer_with_common_metadata(pad, info, u_data, add_post_tracker_timestamp);
    return GST_PAD_PROBE_OK;
}
