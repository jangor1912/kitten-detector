//
// Created by Jan Gorazda on 21/05/2022.
//

#include <gst/gst.h>
#include <glib.h>

#include "inference.h"
#include "utils/utils.h"

GstElement *create_primary_inference_bin(StreamMuxerConfig *config, guint sources_number){
    GstElement *bin = gst_bin_new("primary-inference-bin");

    /* Create all elements */
    GstElement *streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    GstElement *pre_pgie_queue = gst_element_factory_make("queue", "pre-pgie-queue");
    GstElement *pgie = gst_element_factory_make("nvinferserver", "primary-nvinference-engine");
    GstElement *post_pgie_queue = gst_element_factory_make("queue", "post-pgie-queue");

    if(!streammux || !pre_pgie_queue || !pgie || !post_pgie_queue){
        g_printerr("Could not create stream-muxer or primary inference engine!\n");
        return NULL;
    }

    guint batch_size = MIN(config->max_batch_size, sources_number);

    /* Configure all elements */
    g_object_set(G_OBJECT(streammux),
                 "width", config->width,
                 "height", config->height,
                 "batch-size", batch_size,
                 "batched-push-timeout", BATCHED_PUSH_TIMEOUT,
                 "live-source", LIVE_SOURCE,
                 "enable-padding", ENABLE_PADDING,
                 NULL);
    g_object_set (G_OBJECT (pgie),
                  "config-file-path", INFERSERVER_PGIE_CONFIG_FILE,
                  "batch-size", batch_size,
                  NULL);

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                      streammux, pre_pgie_queue,
                     pgie, post_pgie_queue,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many( streammux, pre_pgie_queue, pgie, post_pgie_queue, NULL)) {
        g_printerr("Stream-muxer cannot be linked with inference engine. Exiting.\n");
        return NULL;
    }

    /* Create sink and src ghost pads */
    add_ghost_src_pad_to_bin(bin, post_pgie_queue, "src");

    gchar sink_pad_name[PAD_NAME_LENGTH];
    for(guint i = 0; i < sources_number; i++){
        g_snprintf(sink_pad_name, PAD_NAME_LENGTH, "sink_%d", i);
        add_ghost_sink_pad_to_bin(bin, streammux, sink_pad_name);
    }

    return bin;
}
