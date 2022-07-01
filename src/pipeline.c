//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "../include/pipeline.h"
#include "utils/utils.h"
#include "sources/sources.h"
#include "sinks/sinks.h"
#include "inference/inference.h"
#include "muxer/muxer.h"
#include "handlers/handlers.h"

int run_pipeline(SourcesConfig *sources_config, StreamMuxerConfig *streammux_config) {
    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL;
    gint status = OK;

    /* Standard GStreamer initialization */
    gst_init(NULL, NULL);
    loop = g_main_loop_new(NULL, FALSE);

    /* Create gstreamer elements */
    /* Create Pipeline element that will form a connection of other elements */
    pipeline = gst_pipeline_new("kitten-detector-pipeline");

    if (!pipeline) {
        g_printerr("Pipeline could not be created. Exiting.\n");
        return FAIL;
    }
    g_print("Successfully created the pipeline place-holder!\n");

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);
    g_print("Successfully added bus-watch!\n");

    guint sources_number = sources_config->sources_number;
    GstElement *stream_muxer = create_stream_muxer(streammux_config, sources_number);
    if(!stream_muxer){
        g_printerr("Stream-muxer cannot be created. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline), stream_muxer);
    g_print("Successfully created stream-muxer!\n");

    GstElement *inference_bin = create_primary_inference_bin(streammux_config, sources_number);
    if(!inference_bin){
        g_printerr("Inference bin cannot be created. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline), inference_bin);
    g_print("Successfully created inference-bin!\n");

    gchar sink_pad_name[PAD_NAME_LENGTH];
    gchar tee_src_pad_name[PAD_NAME_LENGTH];
    gchar tee_element_name[ELEMENT_NAME_LENGTH];
    for (guint i = 0; i < sources_number; i++) {
        /* Create source bin */
        gchar* source_uri = sources_config->source_uris[i];
        GstElement *source_bin = create_uridecode_source_bin(i, source_uri);
        if (!source_bin) {
            g_printerr("Failed to create source bin %d with source uri %s. Exiting.\n", i, source_uri);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline), source_bin);
        g_print("Successfully created source-bin (%s)!\n", source_uri);

        /* Create file-sink-bin */
        GstElement *file_sink_bin = create_file_sink_bin(i);
        if(!file_sink_bin){
            g_printerr("Failed to create file-sink-bin %d. Exiting!\n", i);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline), file_sink_bin);
        g_print("Successfully added file-sink-bin %d!\n", i);

        g_print("Trying to solve lacking PTS timestamps issue.\n");
        solve_lacking_pts_timestamps(GST_BIN(file_sink_bin));
        g_print("Successfully solved lacking PTS timestamps issue.\n");

        /* Use TEE element to split stream of frames into two:
         * 1. Frames that are sent to file-sink-bin - for saving high-resolution video files
         * 2. Frames that are sent to stream-muxer - downscaled and passed to inference-engine
         * */

        g_snprintf(tee_element_name, ELEMENT_NAME_LENGTH, "tee_%d", i);
        GstElement *tee = gst_element_factory_make("tee", tee_element_name);
        if(!tee){
            g_print("Tee element for source %d (%s) could not be created. Exiting!\n", i, source_uri);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline), tee);
        g_print("Successfully added tee element (%s) for source '%s' to pipeline!\n", tee_element_name, source_uri);

        /* Connect tee element to source-bin */
        status += connect_two_elements(
                source_bin, tee,
                "sink", "src"
        );
        if(status != OK){
            g_printerr("Cannot connect source-bin (%s) to tee element (%s). Exiting!\n",
                       source_uri, tee_element_name);
            return FAIL;
        }
        g_print("Successfully connected source-bin (%s) to tee element (%s)!\n",
                   source_uri, tee_element_name);

        /* Connect tee element to file-sink-bin */
        g_snprintf(tee_src_pad_name, PAD_NAME_LENGTH, "src_%d", 0);
        status += connect_two_elements(
                tee, file_sink_bin,
                "sink", tee_src_pad_name
        );
        if(status != OK){
            g_printerr("Cannot connect tee (%s) to file-sink-bin. Exiting!\n", source_uri);
            return FAIL;
        }
        g_print("Successfully connected tee (%s) to file-sink-bin. Exiting!\n", source_uri);

        /* Connect tee element to stream-muxer */
        g_snprintf(sink_pad_name, PAD_NAME_LENGTH, "sink_%d", i);
        g_snprintf(tee_src_pad_name, PAD_NAME_LENGTH, "src_%d", 1);
        status += connect_two_elements(
            tee, stream_muxer,
            sink_pad_name, tee_src_pad_name
        );
        if(status != OK){
            g_printerr("Cannot connect source-bin-%d (%s) to stream-muxer. Exiting!\n", i, source_uri);
            return FAIL;
        }
        g_print("Successfully connected source-bin (%s) to stream-muxer!\n", source_uri);
    }

    /* Connect stream-muxer to inference bin */
    status = connect_two_elements(stream_muxer, inference_bin, "sink", "src");
    if(status != OK){
        g_printerr("Cannot connect stream-muxer to inference-bin. Exiting!\n");
        return FAIL;
    }
    g_print("Successfully connected stream-muxer to inference-bin!\n");

    /* Connect inference bin to tiled-display-bin */
    GstElement *sink_bin = create_tilled_display_sink_bin(0, sources_number);
    if (!sink_bin) {
        g_printerr("Failed to create sink bin with tiled display. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline), sink_bin);
    g_print("Successfully created sink-bin!\n");

    /* Connect sink-bin to inference-bin */
    status = connect_two_elements(
            inference_bin, sink_bin, "sink", "src"
    );
    if(status != OK){
        g_printerr("Cannot connect inference-bin with sink-bin. Exiting!\n");
        return FAIL;
    }
    g_print("Successfully connected inference-bin with sink-bin!\n");

    /* Set the pipeline to "playing" state */
    g_print("Now playing:");
    for (gint i = 0; i < sources_number; i++) {
        g_print(" %s,", sources_config->source_uris[i]);
    }
    g_print("\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Wait till pipeline encounters an error or EOS */
    g_print("Running...\n");
    g_main_loop_run(loop);

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);
    return 0;
}
