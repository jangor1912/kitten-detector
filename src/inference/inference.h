//
// Created by Jan Gorazda on 21/05/2022.
//

#ifndef KITTEN_DETECTOR_INFERENCE_H
#define KITTEN_DETECTOR_INFERENCE_H

#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"
#include "structures/structures.h"

/* nv-infer-server configuration options */
#define INFERSERVER_PGIE_CONFIG_FILE  "/models/configs/infer-engine/pgie_config.txt"
#define TRACKER_LIBRARY_PATH "/opt/nvidia/deepstream/deepstream-6.0/lib/libnvds_nvmultiobjecttracker.so"
#define TRACKER_CONFIG_FILE "/models/configs/tracker/config_tracker_NvDCF_accuracy.yml"

GstElement *create_primary_inference_bin(StreamMuxerConfig *config, PipelineData *pipeline_data);

#endif //KITTEN_DETECTOR_INFERENCE_H
