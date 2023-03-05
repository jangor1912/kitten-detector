//
// Created by Jan Gorazda on 31/01/2023.
//

#include "recorder.h"
#include "utils/utils.h"
#include "sinks/sinks.h"
#include "structures/structures.h"
#include "probes/probes.h"

#define RECORDING_BUFFER 2000000000  // 2s of buffer time
#define RECORDER_TEE_NAME "recorder-tee"

GstElement *create_recorder_bin(guint sink_number){
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "recorder-bin-%d", sink_number);
    bin = gst_bin_new(sink_bin_name);

    /* Create all file-sink elements */
    GstElement *pre_recorder_queue = gst_element_factory_make("queue", "pre-recorder-queue");
    GstElement *recorder_tee = gst_element_factory_make("tee", RECORDER_TEE_NAME);
    GstElement *image_sink_bin = create_image_sink_bin(0);
    GstElement *fake_sink_bin = create_fake_sink_bin(0);

    if(!pre_recorder_queue || !recorder_tee || !image_sink_bin || !fake_sink_bin){
        g_printerr("One of the elements of recorder-bin %d could not be created. Exiting!\n", sink_number);
        return NULL;
    }

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                     pre_recorder_queue,
                     recorder_tee,
                     image_sink_bin,
                     fake_sink_bin,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(pre_recorder_queue,
                               recorder_tee,
                               NULL)) {
        g_printerr("Cannot link elements of %d recorder-bin. Exiting.\n", sink_number);
        return NULL;
    }

    connect_two_elements(recorder_tee, fake_sink_bin,
                         "sink", "src_0");
    connect_two_elements(recorder_tee, image_sink_bin,
                         "sink", "src_1");

    /* set buffer on recorder-queue to record few seconds before and after `STOP-RECORDING` signal */
//    g_object_set(G_OBJECT(pre_recorder_queue),
//                "min-threshold-time", RECORDING_BUFFER,
//                "max-size-buffers", 0,
//                "max-size-time", 0,
//                "max-size-bytes", 0,
//                NULL);

    add_ghost_sink_pad_to_bin(bin, pre_recorder_queue, "sink");

    return bin;
}

void start_recording_handler(GstElement *src, GstPad *new_pad, Recorder *recorder) {
    if(recorder->state == Recording) {
        g_print("Recorder is already running - nothing to do here!\n");
        return;
    }

    GstBin *recorder_bin = (GstBin *) recorder->recorder_bin;
    GstElement *recorder_tee = gst_bin_get_by_name(recorder_bin, RECORDER_TEE_NAME);
    if(recorder_tee == NULL){
        g_printerr("Cannot get 'recorder-tee' element from recorder-bin. Exiting!\n");
        return;
    }

    GstPad *recorder_tee_src_pad = gst_element_get_static_pad(recorder_tee, "src_1");
    if(!recorder_tee_src_pad){
        g_printerr("Cannot get source pad of recorder tee!\n");
        return;
    }

    GstElement *image_sink_bin = gst_bin_get_by_name(recorder_bin, "image-sink-bin-0");
    if(image_sink_bin == NULL){
        g_printerr("Cannot get 'image-sink-bin' element from recorder-bin. Exiting!\n");
        return;
    }

    GstPad *image_sink_bin_sink_pad = gst_element_get_static_pad(image_sink_bin, "sink");
    if(image_sink_bin_sink_pad == NULL){
        g_printerr("Cannot retrieve sink-pad of image-sink-bin!\n");
        return;
    }

    if (gst_pad_link(recorder_tee_src_pad, image_sink_bin_sink_pad) != GST_PAD_LINK_OK) {
        g_printerr("Cannot link recorder-tee to image-sink!\n");
        return;
    }
    recorder->state = Recording;
}

void stop_recording_handler(GstElement *src, GstPad *new_pad, Recorder *recorder) {
    if(recorder->state == NotRecording) {
        g_print("Recorder is already unlinked - nothing to do here!\n");
        return;
    }

    GstBin *recorder_bin = (GstBin *) recorder->recorder_bin;
//    GstElement *recorder_tee = gst_bin_get_by_name(recorder_bin, RECORDER_TEE_NAME);
//    if(recorder_tee == NULL){
//        g_printerr("Cannot get 'recorder-tee' element from recorder-bin. Exiting!\n");
//        return;
//    }
//
//    GstPad *recorder_tee_src_pad = gst_element_get_static_pad(recorder_tee, "src_1");
//    if(!recorder_tee_src_pad){
//        g_printerr("Cannot get source pad of recorder tee!\n");
//        return;
//    }

    GstElement *image_sink_bin = gst_bin_get_by_name(recorder_bin, "image-sink-bin-0");
    if(image_sink_bin == NULL){
        g_printerr("Cannot get 'image-sink-bin' element from recorder-bin. Exiting!\n");
        return;
    }

    GstPad *image_sink_bin_sink_pad = gst_element_get_static_pad(image_sink_bin, "sink");
    if(image_sink_bin_sink_pad == NULL){
        g_printerr("Cannot retrieve sink-pad of image-sink-bin!\n");
        return;
    }

//    if (gst_pad_unlink(recorder_tee_src_pad, image_sink_bin_sink_pad) != GST_PAD_LINK_OK) {
//        g_printerr("Cannot link recorder-tee to image-sink!\n");
//        return;
//    }
    g_free(image_sink_bin_sink_pad);
    recorder->state = NotRecording;
}