//
// Created by Jan Gorazda on 21/05/2022.
//

#ifndef KITTEN_DETECTOR_INFERENCE_H
#define KITTEN_DETECTOR_INFERENCE_H

#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"

/* nv-infer-server configuration options */
#define INFERSERVER_PGIE_CONFIG_FILE  "/models/configs/infer-engine/pgie_config.txt"

GstElement *create_primary_inference_bin(StreamMuxerConfig *config, guint sources_number);

#endif //KITTEN_DETECTOR_INFERENCE_H
