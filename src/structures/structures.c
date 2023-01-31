//
// Created by Jan Gorazda on 12/08/2022.
//

#include "structures.h"

Recorder *allocate_recorder();
void deallocate_recorder(Recorder *recorder);

SourceData *allocate_source_data(guint source_id);
void deallocate_source_data(SourceData *elements);

PipelineData *allocate_pipeline_data(guint sources_number){
    PipelineData *elements = (PipelineData*) g_malloc(sizeof(PipelineData));
    if(elements == NULL){
        g_printerr("Pipeline elements cannot be allocated!\n");
        return NULL;
    }
    elements->sources = (SourceData**) g_malloc(sizeof(SourceData*) * sources_number);

    if(elements->sources == NULL){
        g_printerr("Source specific pipeline elements cannot be allocated!\n");
        elements->sources_number = 0;
        deallocate_pipeline_elements(elements);
        return NULL;
    }

    for(guint i=0; i<sources_number; i++){
        elements->sources[i] = allocate_source_data(i);
        if(elements->sources[i] == NULL){
            g_printerr("Cannot allocate source-elements %d!\n", i);
            deallocate_pipeline_elements(elements);
            return NULL;
        }
    }

    elements->pipeline = NULL;
    elements->stream_muxer = NULL;
    elements->inference_bin = NULL;
    elements->osd_sink_bin = NULL;
    elements->sources_number = sources_number;

    return elements;
}


void deallocate_pipeline_elements(PipelineData *elements){
    if(elements == NULL){
        return;
    }

    if(elements->sources_number > 0 && elements->sources != NULL){
        for(guint i=0; i < elements->sources_number; i++){
            deallocate_source_data(elements->sources[i]);
        }
    }

    g_free(elements);
}
SourceData *allocate_source_data(guint source_id){
    SourceData *elements = (SourceData*) g_malloc(sizeof(SourceData));

    if(elements == NULL){
        g_printerr("Cannot allocate source-specific-element!\n");
        return NULL;
    }

    elements->camera_id = source_id;
    elements->frames_passed = 0;

    elements->source_bin = NULL;
    elements->tee = NULL;
    elements->file_sink_bin = NULL;

    elements->recorder = allocate_recorder();

    return elements;
}

void deallocate_source_data(SourceData *elements){
    if(elements == NULL){
        return;
    }

    deallocate_recorder(elements->recorder);

    g_free(elements);
}
Recorder *allocate_recorder(){
    Recorder *recorder = (Recorder*) g_malloc(sizeof(Recorder));

    if(recorder == NULL){
        g_printerr("Cannot allocate recorder!\n");
        return NULL;
    }

    recorder->recorder_bin = NULL;
    recorder->state = Recording;

    return recorder;
}

void deallocate_recorder(Recorder *recorder){
    if(recorder == NULL){
        return;
    }

    recorder->recorder_bin = NULL;

    g_free(recorder);
}