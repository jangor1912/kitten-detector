//
// Created by Jan Gorazda on 14/08/2022.
//

#ifndef KITTEN_DETECTOR_METADATA_H
#define KITTEN_DETECTOR_METADATA_H

#include <time.h>

#include <gst/gst.h>
#include <glib.h>

#include "nvdsmeta.h"

#include "structures/structures.h"

#define KITTEN_DETECTOR_COMMON_GST_META (nvds_get_user_meta_type("KITTEN-DETECTOR.COMMON.GST_USER_META"))

typedef struct {
    guint camera_id;
    long long int frame_number;
    struct timespec *pre_decoder_timestamp;
    struct timespec *post_decoder_timestamp;
    struct timespec *pre_inference_timestamp;
    struct timespec *post_inference_timestamp;
    struct timespec *pre_tracker_timestamp;
    struct timespec *post_tracker_timestamp;
} CommonMetaData;

CommonMetaData *allocate_common_meta_data(SourceData *source_data);
gpointer common_meta_data_copy_func(gpointer data, gpointer user_data);

void common_meta_release_func(gpointer data, gpointer user_data);

gpointer common_gst_to_nvds_meta_transform_func(gpointer data, gpointer user_data);
void common_gst_nvds_meta_release_func(gpointer data, gpointer user_data);

CommonMetaData *get_common_metadata(NvDsUserMeta *user_metadata);


#endif //KITTEN_DETECTOR_METADATA_H
