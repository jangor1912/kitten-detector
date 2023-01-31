//
// Created by Jan Gorazda on 31/01/2023.
//

#include "recorder.h"
#include "utils/utils.h"
#include "sinks/sinks.h"

GstElement *create_recorder_bin(guint sink_number){
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "recorder-bin-%d", sink_number);
    bin = gst_bin_new(sink_bin_name);

    /* Create all file-sink elements */
    GstElement *pre_recorder_queue = gst_element_factory_make("queue", "pre-recorder-queue");
    GstElement *recorder_tee = gst_element_factory_make("tee", "recorder-tee");
    GstElement *image_sink_bin = create_image_sink_bin(sink_number);
    GstElement *fake_sink_bin = create_image_sink_bin(sink_number);

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

    connect_two_elements(recorder_tee, image_sink_bin,
                         "sink", "src_0");
    connect_two_elements(recorder_tee, fake_sink_bin,
                         "sink", "src_1");


    add_ghost_sink_pad_to_bin(bin, pre_recorder_queue, "sink");

    return bin;
}
