//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_PIPELINE_H
#define KITTEN_DETECTOR_PIPELINE_H

typedef struct {
    int sources_number;
    char **source_uris;
} SourcesConfig;

typedef struct {
    int width;
    int height;
    int max_batch_size;
} StreamMuxerConfig;

int run_pipeline(SourcesConfig *sources_config, StreamMuxerConfig *streammux_config);

#endif //KITTEN_DETECTOR_PIPELINE_H
