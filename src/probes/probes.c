//
// Created by Jan Gorazda on 12/08/2022.
//

#include <stdio.h>

#include "gstnvdsmeta.h"
#include "sys/time.h"

#include "probes/probes.h"
#include "utils/utils.h"

#define MAX_DISPLAY_LEN 1024
#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

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


GstPadProbeReturn on_new_buffer_with_common_metadata(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data,
        common_metadata_handler_t common_metadata_handler){

    GstBuffer *buffer = (GstBuffer *) info->data;
    NvDsBatchMeta  *batch_meta = gst_buffer_get_nvds_batch_meta(buffer);
    CommonMetaData *common_meta = NULL;

    if(batch_meta == NULL){
        NvDsMeta *nvds_meta = gst_buffer_get_nvds_meta(buffer);
        if(nvds_meta == NULL){
            g_printerr("Cannot extract neither Nvidia batch meta not Nvidia meta! Deleting probe!");
            return GST_PAD_PROBE_REMOVE;
        }

        if(nvds_meta->meta_type == KITTEN_DETECTOR_COMMON_GST_META && nvds_meta->meta_data != NULL){
            common_meta = (CommonMetaData *)nvds_meta->meta_data;

            if(common_meta == NULL){
                g_printerr("Cannot extract CommonMeta from Nvidia meta! Will try with next buffer!");
            } else {
                common_metadata_handler(common_meta, u_data);
            }

            return GST_PAD_PROBE_OK;
        } else{
            g_printerr("Cannot extract proper CommonMeta from Nvidia meta! "
                       "The meta-type is incorrect! Deleting probe!");
            return GST_PAD_PROBE_REMOVE;
        }
    }

    /* If there is no batch-meta lets see if there is frame-meta */
    NvDsMetaList *l_frame = NULL;
    NvDsUserMeta *user_meta = NULL;
    NvDsMetaList *l_user_meta = NULL;
    NvDsFrameMeta *frame_meta = NULL;

    for(l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next){
        frame_meta = (NvDsFrameMeta *)(l_frame->data);
        if(frame_meta == NULL){
            g_printerr("Cannot extract CommonMeta from buffer! Frame Meta is NULL! "
                       "Will try with another buffer!");
            return GST_PAD_PROBE_OK;
        }

        for(l_user_meta = frame_meta->frame_user_meta_list; l_user_meta != NULL; l_user_meta = l_user_meta->next){
            user_meta = (NvDsUserMeta *)(l_user_meta->data);
            common_meta = get_common_metadata(user_meta);

            if(common_meta != NULL){
                common_metadata_handler(common_meta, u_data);
            }
        }
    }
    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad * pad, GstPadProbeInfo * info, gpointer u_data){
    GstBuffer *buf = (GstBuffer *) info->data;
    NvDsObjectMeta *obj_meta = NULL;
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    NvDsDisplayMeta *display_meta = NULL;
    NvDsUserMeta *user_meta = NULL;
    NvDsMetaList *l_user_meta = NULL;
    NvDsFrameMeta *frame_meta = NULL;
    CommonMetaData *common_meta = NULL;

    long long int frame_number = -1;

    GstClockTime inference_latency = 0;
    GstClockTime tracker_latency = 0;

    int text_offset = 0;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);

    if(batch_meta == NULL){
        NvDsMeta *nvds_meta = gst_buffer_get_nvds_meta(buf);
        if(nvds_meta == NULL){
            g_printerr("Cannot extract neither Nvidia batch meta not Nvidia meta! Deleting probe!");
            return GST_PAD_PROBE_REMOVE;
        }

        if(nvds_meta->meta_type == KITTEN_DETECTOR_COMMON_GST_META && nvds_meta->meta_data != NULL){
            common_meta = (CommonMetaData *)nvds_meta->meta_data;
            frame_number = common_meta->frame_number;
            inference_latency = common_meta->post_inference_timestamp - common_meta->pre_inference_timestamp;
            tracker_latency = common_meta->post_tracker_timestamp - common_meta->pre_tracker_timestamp;

            if(common_meta == NULL){
                g_printerr("Cannot extract CommonMeta from Nvidia meta! Will try with next buffer!");
            }
        } else{
            g_printerr("Cannot extract proper CommonMeta from Nvidia meta! "
                       "The meta-type is incorrect! Deleting probe!");
            return GST_PAD_PROBE_REMOVE;
        }
    }

    gint source_number = 0;

    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        guint num_rects = 0;
        guint vehicle_count = 0;
        guint person_count = 0;
        frame_meta = (NvDsFrameMeta *) (l_frame->data);
        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next) {
            obj_meta = (NvDsObjectMeta *) (l_obj->data);
            if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
                vehicle_count++;
                num_rects++;
            }
            if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
                person_count++;
                num_rects++;
            }
        }

        for(l_user_meta = frame_meta->frame_user_meta_list; l_user_meta != NULL; l_user_meta = l_user_meta->next){
            user_meta = (NvDsUserMeta *)(l_user_meta->data);
            common_meta = get_common_metadata(user_meta);

            if(common_meta != NULL) {
                frame_number = common_meta->frame_number;
                inference_latency = common_meta->post_inference_timestamp - common_meta->pre_inference_timestamp;
                tracker_latency = common_meta->post_tracker_timestamp - common_meta->pre_tracker_timestamp;
            }
        }

        text_offset = 0;
        gchar *display_text = (gchar *) g_malloc0 (MAX_DISPLAY_LEN);
        text_offset += snprintf(display_text + text_offset,
                                MAX_DISPLAY_LEN, "Source = %d ", source_number);
        text_offset += snprintf(display_text + text_offset,
                                MAX_DISPLAY_LEN, "Frame = %lld\n", frame_number);
        text_offset += snprintf(display_text + text_offset,
                                MAX_DISPLAY_LEN, "Person = %d ", person_count);
        text_offset += snprintf(display_text + text_offset ,
                                MAX_DISPLAY_LEN, "Vehicle = %d\n", vehicle_count);
        text_offset += snprintf(display_text + text_offset ,
                                MAX_DISPLAY_LEN, "Inference latency = %7.02f ms (%lu ns)\n",
                                inference_latency / 1000000.0, inference_latency);
        text_offset += snprintf(display_text + text_offset ,
                                MAX_DISPLAY_LEN, "Tracking latency = %7.02f ms (%lu ns)\n",
                                tracker_latency / 1000000.0, tracker_latency);

        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
        NvOSD_TextParams *txt_params  = &display_meta->text_params[source_number];
        display_meta->num_labels = 1;
        txt_params->display_text = display_text;

        /* Now set the offsets where the string should appear */
        txt_params->x_offset = 10;
        txt_params->y_offset = 12;

        /* Font , font-color and font-size */
        txt_params->font_params.font_name = "Serif";
        txt_params->font_params.font_size = 10;
        txt_params->font_params.font_color.red = 1.0;
        txt_params->font_params.font_color.blue = 1.0;
        txt_params->font_params.font_color.alpha = 1.0;

        /* Text background color */
        txt_params->set_bg_clr = 1;
        txt_params->text_bg_clr.red = 0.0;
        txt_params->text_bg_clr.green = 0.0;
        txt_params->text_bg_clr.blue = 0.0;
        txt_params->text_bg_clr.alpha = 1.0;

        nvds_add_display_meta_to_frame(frame_meta, display_meta);

        source_number++;
    }

    return GST_PAD_PROBE_OK;
}
