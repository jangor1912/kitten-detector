//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>
#include "sinks.h"

#define ELEMENT_NAME_LENGTH 16

#define TILED_OUTPUT_WIDTH 1280
#define TILED_OUTPUT_HEIGHT 720

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 0

/* By default, OSD will not display text. To display text, change this to 1 */
#define OSD_DISPLAY_TEXT 0


GstElement *create_sink_bin(gchar *sink_bin_name) {
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    bin = gst_bin_new(sink_bin_name);

    /* Use nvtiler to composite the batched frames into a 2D tiled array based on the source of the frames. */
    GstElement *before_tiler_queue = gst_element_factory_make("queue", "before-tiler-queue");
    GstElement *tiler = gst_element_factory_make("nvmultistreamtiler", "nvtiler");

    /* Use convertor to convert from NV12 to RGBA as required by nvosd */
    GstElement *before_converter_queue = gst_element_factory_make("queue", "before-converter-queue");
    GstElement *nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");

    /* Create OSD to draw on the converted RGBA buffer */
    GstElement *before_osd_queue = gst_element_factory_make("queue", "before-osd-queue");
    GstElement *nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");

    /* Finally, render the osd output */
    GstElement *before_renderer_queue = gst_element_factory_make("queue", "before-renderer-queue");
    GstElement *sink = gst_element_factory_make("nveglglessink", "nvvideo-renderer");

    if (!before_tiler_queue || !tiler
        || !before_converter_queue || !nvvidconv
        || !before_osd_queue || !nvosd
        || !before_renderer_queue || !sink) {
        g_printerr("One element of sink could not be created. Exiting.\n");
        return NULL;
    }

    /* Configure added elements */
    guint tiler_rows = (guint) sqrt(num_sources);
    guint tiler_columns = (guint) ceil(1.0 * num_sources / tiler_rows);
    /* we set the tiler properties here */
    g_object_set(G_OBJECT(tiler),
                 "rows", tiler_rows,
                 "columns", tiler_columns,
                 "width", TILED_OUTPUT_WIDTH,
                 "height", TILED_OUTPUT_HEIGHT,
                 NULL);

    g_object_set(G_OBJECT(nvosd),
                 "process-mode", OSD_PROCESS_MODE,
                 "display-text", OSD_DISPLAY_TEXT,
                 NULL);

    g_object_set(G_OBJECT(sink),
                 "qos", 0,
                 NULL);

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                     before_tiler_queue, tiler,
                     before_converter_queue, nvvidconv,
                     before_osd_queue, nvosd,
                     before_renderer_queue, sink,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(before_tiler_queue, tiler,
                               before_converter_queue, nvvidconv,
                               before_osd_queue, nvosd,
                               before_renderer_queue, sink,
                               NULL)) {
        g_printerr("Elements could not be linked. Exiting.\n");
        return NULL;
    }
}
