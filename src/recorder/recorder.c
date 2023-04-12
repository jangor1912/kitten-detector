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

void start_recording_handler(GstElement *src, GstPad *new_pad, gpointer recorder_data) {
    // g_print("Reached: 'start_recording_handler'\n");

    Recorder *recorder = (Recorder*) recorder_data;

    if(recorder == NULL){
        g_printerr("Recorder is NULL in 'start_recording_handler'. Exiting!\n");
        return;
    }

    if(recorder->recorder_bin == NULL){
        g_printerr("Recorder-Bin is NULL in 'start_recording_handler'. Exiting!\n");
        return;
    }

    if(recorder->state == Recording) {
        // g_print("Recorder is already running - nothing to do here!\n");
        return;
    }

    recorder->state = Recording;
    g_print("Successfully changed Recorder state to Recording\n");

    g_print("Trying to add linking probe to recorder\n");
    GstElement *recorder_tee = gst_bin_get_by_name(GST_BIN(recorder->recorder_bin), RECORDER_TEE_NAME);
    if(recorder_tee == NULL){
        g_printerr("Cannot get 'recorder-tee' element from recorder-bin. Exiting!\n");
        return;
    }

    GstPad *recorder_tee_src_pad = gst_element_get_static_pad(recorder_tee, "src_1");
    if(!recorder_tee_src_pad){
        g_printerr("Cannot get source pad of recorder tee!\n");
        return;
    }
    gst_pad_add_probe (recorder_tee_src_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                       start_recording_probe_callback,
                       (gpointer) recorder->recorder_bin, NULL);
    g_print("Successfully added 'start_recording_probe_callback' to 'recorder_tee_src_pad'\n");

}

void stop_recording_handler(GstElement *src, GstPad *new_pad, gpointer recorder_data) {
    // g_print("Reached: 'stop_recording_handler'\n");

    Recorder *recorder = (Recorder*) recorder_data;

    if(recorder == NULL){
        g_printerr("Recorder is NULL in 'stop_recording_handler'. Exiting!\n");
        return;
    }

    if(recorder->recorder_bin == NULL){
        g_printerr("Recorder-Bin is NULL in 'stop_recording_handler'. Exiting!\n");
        return;
    }

    if(recorder->state == NotRecording) {
        // g_print("Recorder is already unlinked - nothing to do here!\n");
        return;
    }

    recorder->state = NotRecording;
    g_print("Successfully changed Recorder state to NotRecording\n");

    g_print("Trying to add linking probe to recorder\n");
    GstElement *recorder_tee = gst_bin_get_by_name(GST_BIN(recorder->recorder_bin), RECORDER_TEE_NAME);
    if(recorder_tee == NULL){
        g_printerr("Cannot get 'recorder-tee' element from recorder-bin. Exiting!\n");
        return;
    }

    GstPad *recorder_tee_src_pad = gst_element_get_static_pad(recorder_tee, "src_1");
    if(!recorder_tee_src_pad){
        g_printerr("Cannot get source pad of recorder tee!\n");
        return;
    }
    gst_pad_add_probe (recorder_tee_src_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                       stop_recording_probe_callback,
                       (gpointer) recorder->recorder_bin, NULL);
    g_print("Successfully added 'stop_recording_probe_callback' to 'recorder_tee_src_pad'\n");
}