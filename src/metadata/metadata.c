//
// Created by Jan Gorazda on 14/08/2022.
//

#include "gstnvdsmeta.h"
#include "utils/utils.h"

#include "metadata.h"

CommonMetaData *allocate_common_meta_data(SourceData *source_data){
    CommonMetaData *meta_data = (CommonMetaData *) g_malloc(sizeof(CommonMetaData));

    if(meta_data == NULL){
        g_printerr("Cannot allocate common-meta-data from source %d, frame = %d.\n",
                   source_data->camera_id, (int) source_data->frames_passed);
        return NULL;
    }

    meta_data->camera_id = source_data->camera_id;
    meta_data->frame_number = source_data->frames_passed;
    meta_data->pre_decoder_timestamp = 0;
    meta_data->post_decoder_timestamp = 0;
    meta_data->pre_inference_timestamp = 0;
    meta_data->post_inference_timestamp = 0;
    meta_data->pre_tracker_timestamp = 0;
    meta_data->post_tracker_timestamp = 0;

    return meta_data;
}

CommonMetaData *copy_common_metadata(CommonMetaData *src_meta){
    CommonMetaData *dst_meta = (CommonMetaData *) g_malloc(sizeof(CommonMetaData));

    if(dst_meta == NULL){
        g_printerr("Cannot copy common-meta-data from source %d, frame = %d.\n",
                   src_meta->camera_id, (int) src_meta->frame_number);
        return NULL;
    }

    dst_meta->camera_id = src_meta->camera_id;
    dst_meta->frame_number = src_meta->frame_number;
    dst_meta->pre_decoder_timestamp = src_meta->pre_decoder_timestamp;
    dst_meta->post_decoder_timestamp = src_meta->post_decoder_timestamp;
    dst_meta->pre_inference_timestamp = src_meta->pre_inference_timestamp;
    dst_meta->post_inference_timestamp = src_meta->post_inference_timestamp;
    dst_meta->pre_tracker_timestamp = src_meta->pre_tracker_timestamp;
    dst_meta->post_tracker_timestamp = src_meta->post_tracker_timestamp;

    return dst_meta;
}

gpointer common_meta_data_copy_func(gpointer data, gpointer user_data) {
    CommonMetaData *src_meta = (CommonMetaData *)data;
    CommonMetaData *dst_meta = copy_common_metadata(src_meta);
    return (gpointer)dst_meta;
}

void release_common_meta_data(CommonMetaData *common_meta_data) {
    free(common_meta_data);
    common_meta_data = NULL;
}

void common_meta_release_func(gpointer data, gpointer user_data) {
    CommonMetaData *common_meta_data = (CommonMetaData *)data;
    release_common_meta_data(common_meta_data);
}

gpointer common_gst_to_nvds_meta_transform_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *)data;
    CommonMetaData *src_meta = (CommonMetaData *)user_meta->user_meta_data;
    CommonMetaData *dst_meta = copy_common_metadata(src_meta);
    return (gpointer)dst_meta;
}

void common_gst_nvds_meta_release_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *)data;
    CommonMetaData *common_meta_data = (CommonMetaData *)user_meta->user_meta_data;
    g_free(common_meta_data);
}

CommonMetaData *get_common_metadata(NvDsUserMeta *user_metadata){
    if(user_metadata->base_meta.meta_type == KITTEN_DETECTOR_COMMON_GST_META){
        CommonMetaData *common_meta = (CommonMetaData *)(user_metadata->user_meta_data);
        return common_meta;
    }
    return NULL;
}