//
// Created by jan on 12.06.22.
//

#ifndef KITTEN_DETECTOR_MUXER_H
#define KITTEN_DETECTOR_MUXER_H

/* stream-muxer configuration options */
#define BATCHED_PUSH_TIMEOUT 10000
#define LIVE_SOURCE 1
#define ENABLE_PADDING 1

#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"

GstElement *create_stream_muxer(StreamMuxerConfig *config, guint sources_number);

#endif //KITTEN_DETECTOR_MUXER_H
