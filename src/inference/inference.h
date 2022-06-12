//
// Created by Jan Gorazda on 21/05/2022.
//

#ifndef KITTEN_DETECTOR_INFERENCE_H
#define KITTEN_DETECTOR_INFERENCE_H

#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"

/* stream-muxer configuration options */
#define BATCHED_PUSH_TIMEOUT 10000
#define LIVE_SOURCE 1
#define ENABLE_PADDING 1

/* nv-infer-server configuration options */
#define INFERSERVER_PGIE_CONFIG_FILE  "/project/configs/inferserver/pgie_config.txt"

GstElement *create_primary_inference_bin(StreamMuxerConfig *config, guint sources_number);

#endif //KITTEN_DETECTOR_INFERENCE_H
