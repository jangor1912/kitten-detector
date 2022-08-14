//
// Created by Jan Gorazda on 12/08/2022.
//

#ifndef KITTEN_DETECTOR_STRUCTURES_H
#define KITTEN_DETECTOR_STRUCTURES_H

#include <gst/gst.h>
#include <glib.h>

enum PipelineState {Recording = 1, NotRecording = 0};

typedef struct {
    time_t recording_start_time;
    time_t recording_stop_time;
} RecordingManager;

typedef struct {
    GstElement *source_bin;
    GstElement *tee;
    GstElement *file_sink_bin;

    guint camera_id;
    long long int frames_passed;
} SourceData;

typedef struct {
    GstElement *pipeline;

    GstElement *stream_muxer;
    GstElement *inference_bin;
    GstElement *osd_sink_bin;

    SourceData **sources;
    guint sources_number;

    enum PipelineState state;
    RecordingManager *recording_manager;
} PipelineData;

PipelineData *allocate_pipeline_data(guint sources_number);
void deallocate_pipeline_elements(PipelineData *elements);

#endif //KITTEN_DETECTOR_STRUCTURES_H
