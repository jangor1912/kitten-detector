//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "../include/pipeline.h"

#define CHAR_ARRAY_LENGTH 16

gint run_pipeline(gchar **source_uris, gint source_uris_number) {
    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL;
    GstElement *streammux = NULL;
    GstElement *sink = NULL;

    /* Standard GStreamer initialization */
    gst_init();
    loop = g_main_loop_new(NULL, FALSE);

    /* Create gstreamer elements */
    /* Create Pipeline element that will form a connection of other elements */
    pipeline = gst_pipeline_new("kitten-detector-pipeline");

    /* Create nvstreammux instance to form batches from one or more sources. */
    streammux = gst_element_factory_make("nvstreammux", "stream-muxer");

    if (!pipeline || !streammux) {
        g_printerr("One element could not be created. Exiting.\n");
        return -1;
    }
    gst_bin_add(GST_BIN(pipeline), streammux);

    for (guint i = 0; i < source_uris_number; i++) {
        GstPad *sinkpad, *srcpad;
        gchar pad_name[CHAR_ARRAY_LENGTH] = {};
        gsnprintf(pad_name, CHAR_ARRAY_LENGTH-1, "sink-bin-%02d");
        GstElement *source_bin = create_source_bin(i, );

        if (!source_bin) {
            g_printerr("Failed to create source bin. Exiting.\n");
            return -1;
        }

        gst_bin_add(GST_BIN(pipeline), source_bin);

        g_snprintf(pad_name, 15, "sink_%u", i);
        sinkpad = gst_element_get_request_pad(streammux, pad_name);
        if (!sinkpad) {
            g_printerr("Streammux request sink pad failed. Exiting.\n");
            return -1;
        }

        srcpad = gst_element_get_static_pad(source_bin, "src");
        if (!srcpad) {
            g_printerr("Failed to get src pad of source bin. Exiting.\n");
            return -1;
        }

        if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
            g_printerr("Failed to link source bin to stream muxer. Exiting.\n");
            return -1;
        }

        gst_object_unref(srcpad);
        gst_object_unref(sinkpad);
    }

    /* Set the pipeline to "playing" state */
    g_print("Now playing:");
    for (i = 0; i < num_sources; i++) {
        g_print(" %s,", argv[i + 1]);
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
