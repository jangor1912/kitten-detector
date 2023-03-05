//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "pipeline.h"
#include "structures/structures.h"
#include "probes/probes.h"
#include "utils/utils.h"
#include "sources/sources.h"
#include "sinks/sinks.h"
#include "inference/inference.h"
#include "muxer/muxer.h"
#include "handlers/handlers.h"
#include "recorder/recorder.h"

#define SAVE_VIDEO 0

int run_pipeline(SourcesConfig *sources_config, StreamMuxerConfig *streammux_config) {
    GMainLoop *loop = NULL;
    PipelineData *pipeline_data = allocate_pipeline_data(sources_config->sources_number);
    gint status = OK;

    /* Standard GStreamer initialization */
    gst_init(NULL, NULL);
    loop = g_main_loop_new(NULL, FALSE);

    /* Create gstreamer pipeline_data */
    /* Create Pipeline element that will form a connection of other pipeline_data */
    pipeline_data->pipeline = gst_pipeline_new("kitten-detector-pipeline");

    if (!pipeline_data->pipeline) {
        g_printerr("Pipeline could not be created. Exiting.\n");
        return FAIL;
    }
    g_print("Successfully created the pipeline place-holder!\n");

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline_data->pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);
    g_print("Successfully added bus-watch!\n");

    guint sources_number = sources_config->sources_number;
    pipeline_data->stream_muxer = create_stream_muxer(streammux_config, sources_number);
    if(!pipeline_data->stream_muxer){
        g_printerr("Stream-muxer cannot be created. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline_data->pipeline), pipeline_data->stream_muxer);
    g_print("Successfully created stream-muxer!\n");

    pipeline_data->inference_bin = create_primary_inference_bin(streammux_config, sources_number);
    if(!pipeline_data->inference_bin){
        g_printerr("Inference bin cannot be created. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline_data->pipeline), pipeline_data->inference_bin);
    g_print("Successfully created inference-bin!\n");

    gchar sink_pad_name[PAD_NAME_LENGTH];
    gchar tee_src_pad_name[PAD_NAME_LENGTH];
    gchar tee_element_name[ELEMENT_NAME_LENGTH];
    for (guint i = 0; i < sources_number; i++) {
        /* Create source bin */
        SourceData *source_data = pipeline_data->sources[i];
        gchar* source_uri = sources_config->source_uris[i];
        source_data->source_bin = create_source_bin(i, source_uri, NULL);
        if (!source_data->source_bin) {
            g_printerr("Failed to create source bin %d with source uri %s. Exiting.\n", i, source_uri);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline_data->pipeline), source_data->source_bin);
        g_print("Successfully created source-bin (%s)!\n", source_uri);

        /* Use TEE element to split stream of frames into two:
         * 1. Frames that are sent to stream-muxer - downscaled and passed to inference-engine
         * 2. Frames that are sent to file-sink-bin - for saving high-resolution video files
         * */

        g_snprintf(tee_element_name, ELEMENT_NAME_LENGTH, "tee_%d", i);
        source_data->tee = gst_element_factory_make("tee", tee_element_name);
        if(!source_data->tee){
            g_print("Tee element for source %d (%s) could not be created. Exiting!\n", i, source_uri);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline_data->pipeline), source_data->tee);
        g_print("Successfully added tee element (%s) for source '%s' to pipeline!\n", tee_element_name, source_uri);

        /* Connect tee element to source-bin */
        status += connect_two_elements(
                source_data->source_bin, source_data->tee,
                "sink", "src"
        );
        if(status != OK){
            g_printerr("Cannot connect source-bin (%s) to tee element (%s). Exiting!\n",
                       source_uri, tee_element_name);
            return FAIL;
        }
        g_print("Successfully connected source-bin (%s) to tee element (%s)!\n",
                   source_uri, tee_element_name);

        /* Connect tee element to stream-muxer */
        g_snprintf(sink_pad_name, PAD_NAME_LENGTH, "sink_%d", i);
        g_snprintf(tee_src_pad_name, PAD_NAME_LENGTH, "src_%d", 0);
        status += connect_two_elements(
                source_data->tee, pipeline_data->stream_muxer,
                sink_pad_name, tee_src_pad_name
        );
        if(status != OK){
            g_printerr("Cannot connect source-bin-%d (%s) to stream-muxer. Exiting!\n", i, source_uri);
            return FAIL;
        }
        g_print("Successfully connected source-bin (%s) to stream-muxer!\n", source_uri);

        if(SAVE_VIDEO){
            /* Create file-sink-bin */
            source_data->file_sink_bin = create_file_sink_bin(i);
            if(!source_data->file_sink_bin){
                g_printerr("Failed to create file-sink-bin %d. Exiting!\n", i);
                return FAIL;
            }
            gst_bin_add(GST_BIN(pipeline_data->pipeline), source_data->file_sink_bin);
            g_print("Successfully added file-sink-bin %d!\n", i);

            /* Connect tee element to file-sink-bin */
            g_snprintf(tee_src_pad_name, PAD_NAME_LENGTH, "src_%d", 1);
            status += connect_two_elements(
                    source_data->tee, source_data->file_sink_bin,
                    "sink", tee_src_pad_name
            );
            if(status != OK){
                g_printerr("Cannot connect tee (%s) to file-sink-bin. Exiting!\n", source_uri);
                return FAIL;
            }
            g_print("Successfully connected tee (%s) to file-sink-bin. Exiting!\n", source_uri);
        } else {
            /* Create recorder */
            Recorder *recorder = source_data->recorder;
            if(recorder == NULL){
                g_printerr("Failed to retrieve Recorder (%s) from source-data. Exiting!\n", source_uri);
                return FAIL;
            }

            GstElement *recorder_bin = create_recorder_bin(i);
            recorder->recorder_bin = recorder_bin;
            recorder->state = Recording;

            /* Register the "start-recording" and "stop-recording" signals on the pipeline */
            g_signal_new("start-recording", G_TYPE_FROM_INSTANCE(recorder_bin),
                         G_SIGNAL_RUN_LAST, 0,
                         NULL, NULL,
                         NULL, G_TYPE_NONE, 0);
            g_signal_new("stop-recording", G_TYPE_FROM_INSTANCE(recorder_bin),
                         G_SIGNAL_RUN_LAST, 0,
                         NULL, NULL,
                         NULL, G_TYPE_NONE, 0);
            g_print("Successfully registered 'start-recording' and 'stop-recording' signals for %d source.\n", i);

            g_signal_connect(recorder_bin, "stop-recording", G_CALLBACK (stop_recording_handler), &recorder);
            g_signal_connect(recorder_bin, "start-recording", G_CALLBACK (stop_recording_handler), &recorder);

            gst_bin_add(GST_BIN(pipeline_data->pipeline), recorder_bin);

            g_print("Successfully added recorder-bin %d!\n", i);

            /* Connect tee element to recorder-bin */
            g_snprintf(tee_src_pad_name, PAD_NAME_LENGTH, "src_%d", 1);
            status += connect_two_elements(
                    source_data->tee, source_data->recorder->recorder_bin,
                    "sink", tee_src_pad_name
            );
            if(status != OK){
                g_printerr("Cannot connect tee (%s) to recorder-bin. Exiting!\n", source_uri);
                return FAIL;
            }
            g_print("Successfully connected tee (%s) to recorder-bin. Exiting!\n", source_uri);

        }

        /* Add common-meta-data attaching probe to the source-pad of source-bin */
        GstPad *source_bin_src_pad = gst_element_get_static_pad(source_data->source_bin, "src");
        if(!source_bin_src_pad){
            g_printerr("Cannot get source pad of source bin number %d!\n", i);
            return FAIL;
        }
        gst_pad_add_probe(
                source_bin_src_pad, GST_PAD_PROBE_TYPE_BUFFER,
                attach_common_meta_data_to_buffer_probe,
                (gpointer)source_data, NULL);
        gst_object_unref(source_bin_src_pad);
    }

    /* Connect stream-muxer to inference bin */
    status = connect_two_elements(pipeline_data->stream_muxer, pipeline_data->inference_bin,
                                  "sink", "src");
    if(status != OK){
        g_printerr("Cannot connect stream-muxer to inference-bin. Exiting!\n");
        return FAIL;
    }
    g_print("Successfully connected stream-muxer to inference-bin!\n");

    /* Connect inference bin to tiled-display-bin */
    pipeline_data->osd_sink_bin = create_tilled_display_sink_bin(0, sources_number);
    if (!pipeline_data->osd_sink_bin) {
        g_printerr("Failed to create sink bin with tiled display. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline_data->pipeline), pipeline_data->osd_sink_bin);
    g_print("Successfully created sink-bin!\n");

    /* Connect sink-bin to inference-bin */
    status = connect_two_elements(
            pipeline_data->inference_bin, pipeline_data->osd_sink_bin,
            "sink", "src"
    );
    if(status != OK){
        g_printerr("Cannot connect inference-bin with sink-bin. Exiting!\n");
        return FAIL;
    }
    g_print("Successfully connected inference-bin with osd-sink-bin!\n");

    /* Set the pipeline to "playing" state */
    g_print("Now playing:");
    for (gint i = 0; i < sources_number; i++) {
        g_print(" %s,", sources_config->source_uris[i]);
    }
    g_print("\n");
    gst_element_set_state(pipeline_data->pipeline, GST_STATE_PLAYING);

    /* Wait till pipeline encounters an error or EOS */
    g_print("Running...\n");
    g_main_loop_run(loop);

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline_data->pipeline, GST_STATE_NULL);
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(pipeline_data->pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);
    deallocate_pipeline_elements(pipeline_data);
    return 0;
}
