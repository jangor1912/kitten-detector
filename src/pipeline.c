//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "../include/pipeline.h"
#include "utils/utils.h"
#include "sources/sources.h"
#include "inference/inference.h"
#include "handlers/handlers.h"

guint run_pipeline(SourcesConfig *sources_config, StreamMuxerConfig *streammux_config) {
    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL;
    GstElement *streammux = NULL;
    GstElement *sink = NULL;

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

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    guint bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    guint sources_number = sources_config->sources_number;
    GstElement *inference_bin = create_primary_inference_bin(streammux_config, sources_number);
    if(!inference_bin){
        g_printerr("Inference bin cannot be created. Exiting!\n");
        return FAIL;
    }
    gst_bin_add(GST_BIN(pipeline), inference_bin);

    gchar sink_pad_name[PAD_NAME_LENGTH];
    for (guint i = 0; i < sources_number; i++) {
        /* Create source bin */
        gchar* source_uri = sources_config->source_uris[i];
        GstElement *source_bin = create_uridecode_source_bin(i, source_uri);
        if (!source_bin) {
            g_printerr("Failed to create source bin %d with source uri %s. Exiting.\n", i, source_uri);
            return FAIL;
        }
        gst_bin_add(GST_BIN(pipeline), source_bin);

        /* Connect source-bin to inference-bin */
        g_snprintf(sink_pad_name, PAD_NAME_LENGTH, "sink_%d", i);
        gint status = connect_two_elements(
            source_bin, inference_bin,
            sink_pad_name, "src"
        );
        if(status != OK){
            g_printerr("Cannot connect source-bin-%d (%s) to inference-bin. Exiting!\n", i, source_uri);
            return FAIL;
        }
    }

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
