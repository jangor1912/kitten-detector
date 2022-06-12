//
// Created by jan on 12.06.22.
//

#include "muxer.h"

GstElement *create_stream_muxer(StreamMuxerConfig *config, guint sources_number){
    GstElement *streammux = gst_element_factory_make("nvstreammux", "stream-muxer");

    if(!streammux){
        g_printerr("Could not create stream-muxer!\n");
        return NULL;
    }

    guint batch_size = MIN(config->max_batch_size, sources_number);

    g_object_set(G_OBJECT(streammux),
                 "width", config->width,
                 "height", config->height,
                 "batch-size", batch_size,
                 "batched-push-timeout", BATCHED_PUSH_TIMEOUT,
                 "live-source", LIVE_SOURCE,
                 "enable-padding", ENABLE_PADDING,
                 NULL);

    return streammux;
}