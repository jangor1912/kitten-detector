//
// Created by Jan Gorazda on 12/08/2022.
//

#include "gstnvdsmeta.h"

#include "probes.h"
#include "metadata/metadata.h"

GstPadProbeReturn stop_recording_probe_callback(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data){
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn start_recording_probe_callback(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data){
    return GST_PAD_PROBE_OK;
}


GstPadProbeReturn attach_common_meta_data_to_buffer_probe(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data) {
    GstBuffer *buffer = (GstBuffer *)info->data;
    NvDsMeta *meta = NULL;

    SourceData *source_data = (SourceData *)u_data;

    /* Initialize common metadata */
    allocate_common_meta_data(source_data);
    CommonMetaData *common_meta_data = allocate_common_meta_data(source_data);
    if (common_meta_data == NULL) {
        return GST_FLOW_ERROR;
    }

    /* New frame appeared increase source connector frame number */
    source_data->frames_passed += 1;

    /* Attach decoder metadata to gst buffer using gst_buffer_add_nvds_meta() */
    meta = gst_buffer_add_nvds_meta(
            buffer, common_meta_data, NULL,
            common_meta_data_copy_func,
            common_meta_release_func
    );

    /* Set metadata type */
    meta->meta_type = (GstNvDsMetaType)KITTEN_DETECTOR_COMMON_GST_META;

    /* Set transform function to transform metadata from Gst meta to nvds meta */
    meta->gst_to_nvds_meta_transform_func = common_gst_to_nvds_meta_transform_func;

    /* Set release function to release the transformed nvds metadata */
    meta->gst_to_nvds_meta_release_func = common_gst_nvds_meta_release_func;

    return GST_PAD_PROBE_OK;
}