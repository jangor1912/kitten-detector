//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_PIPELINE_H
#define KITTEN_DETECTOR_PIPELINE_H

#include "sources/sources.h"
#include "inference/inference.h"

guint run_pipeline(SourcesConfig *sources_config, StreamMuxerConfig *streammux_config);

#endif //KITTEN_DETECTOR_PIPELINE_H
