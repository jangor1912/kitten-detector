//
// Created by Jan Gorazda on 21/05/2022.
//

#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"
#include "inference.h"
#include "utils/utils.h"

GstElement *create_primary_inference_bin(StreamMuxerConfig *config, guint sources_number){
    GstElement *bin = gst_bin_new("primary-inference-bin");

    /* Create all elements */
    GstElement *pre_pgie_queue = gst_element_factory_make("queue", "pre-pgie-queue");
    GstElement *pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
    GstElement *nvtracker = gst_element_factory_make ("nvtracker", "nvtracker");
    GstElement *post_pgie_queue = gst_element_factory_make("queue", "post-pgie-queue");

    if(!pre_pgie_queue || !pgie || !nvtracker || !post_pgie_queue){
        g_printerr("Could not create primary inference engine!\n");
        return NULL;
    }

    guint batch_size = MIN(config->max_batch_size, sources_number);

    /* Configure all elements */
    g_object_set (G_OBJECT (pgie),
                  "config-file-path", INFERSERVER_PGIE_CONFIG_FILE,
                  "batch-size", batch_size,
                  NULL);
    /* Configure the nvtracker element for using the particular tracker algorithm. */
    g_object_set (G_OBJECT (nvtracker),
                  "ll-lib-file", TRACKER_LIBRARY_PATH,
                  "ll-config-file", TRACKER_CONFIG_FILE,
                  "tracker-width", 640, "tracker-height", 480,
                  NULL);

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                      pre_pgie_queue,
                     pgie,
                     nvtracker,
                     post_pgie_queue,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(pre_pgie_queue, pgie, nvtracker, post_pgie_queue, NULL)) {
        g_printerr("Elements of inference-bin cannot be linked together. Exiting.\n");
        return NULL;
    }

    /* Create sink and src ghost pads */
    add_ghost_src_pad_to_bin(bin, post_pgie_queue, "src");
    add_ghost_sink_pad_to_bin(bin, pre_pgie_queue, "sink");

    return bin;
}
