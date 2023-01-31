//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sinks.h"
#include "utils/utils.h"
#include "probes/probes.h"

#define TILED_OUTPUT_WIDTH 1920
#define TILED_OUTPUT_HEIGHT 1080

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 1
#define OSD_DISPLAY_TEXT 1
#define OSD_DISPLAY_BBOX 1
#define OSD_DISPLAY_MASK 1


GstElement *create_tilled_display_sink_bin(guint sink_bin_number, guint sinks_number) {
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "osd-sink-bin-%d", sink_bin_number);
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
    guint tiler_rows = (guint) sqrt(sinks_number);
    guint tiler_columns = (guint) ceil(1.0 * sinks_number / tiler_rows);
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
                 "display-bbox", OSD_DISPLAY_BBOX,
                 "display-mask", OSD_DISPLAY_MASK,
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

    add_ghost_sink_pad_to_bin(bin, before_tiler_queue, "sink");

    /* Set probe to render proper text after OSD */
    GstPad *osd_sink_pad = gst_element_get_static_pad(nvosd, "sink");
    if(!osd_sink_pad){
        g_printerr("Cannot get sink pad of osd element!\n");
        return NULL;
    }
    gst_pad_add_probe(
            osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
            osd_sink_pad_buffer_probe,
            NULL, NULL
    );
    gst_object_unref(osd_sink_pad);

    return bin;
}

GstElement *create_file_sink_bin(guint sink_number){
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "file-sink-bin-%d", sink_number);
    bin = gst_bin_new(sink_bin_name);

    /* Create all file-sink elements */
    GstElement *before_sink_queue = gst_element_factory_make("queue", "before-file-sink-queue");
    GstElement *converter = gst_element_factory_make("nvvideoconvert", "file-sink-video-converter");
    GstElement *encoder = gst_element_factory_make("nvv4l2h264enc", "file-sink-video-encoder");
    GstElement *parser = gst_element_factory_make("h264parse", "file-sink-parser");
    GstElement *file_sink = gst_element_factory_make("splitmuxsink", "file-sink");

    if(!before_sink_queue || !converter || !encoder || !parser || !file_sink){
        g_printerr("One of the elements of file-sink %d could not be created. Exiting!\n", sink_number);
        return NULL;
    }

    /* Configure file-sink to create 1 min long files and save it to proper directory */
    gchar output_directory[MAX_DIRECTORY_PATH_SIZE];
    gchar output_file_format[MAX_DIRECTORY_PATH_SIZE];
    g_snprintf(output_directory, MAX_DIRECTORY_PATH_SIZE,
               "%s/%s", FILE_SINK_DIRECTORY, sink_bin_name);
    g_snprintf(output_file_format, MAX_DIRECTORY_PATH_SIZE,
               "%s/%s", output_directory, FILE_NAME_FORMAT);

    /* Create directory and delete all files in it */
    struct stat st = {0};
    if (stat(output_directory, &st) == -1) {
        mkdir(output_directory, 0777);
    }
    delete_all_files_in_directory(output_directory);

    g_object_set(G_OBJECT(file_sink),
                 "max-size-time", 60000000000,
                 "location", output_file_format,
                 "async-finalize", TRUE,
                 NULL);

    g_object_set(G_OBJECT(parser),
                 "config-interval", 1,
                 NULL);


    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                     before_sink_queue,
                     converter,
                     encoder,
                     parser,
                     file_sink,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(before_sink_queue,
                               converter,
                               encoder,
                               parser,
                               file_sink,
                               NULL)) {
        g_printerr("Cannot link elements of %d file-sink-bin. Exiting.\n", sink_number);
        return NULL;
    }

    add_ghost_sink_pad_to_bin(bin, before_sink_queue, "sink");

    return bin;
}

GstElement *create_image_sink_bin(guint sink_number){
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "image-sink-bin-%d", sink_number);
    bin = gst_bin_new(sink_bin_name);

    /* Create all file-sink elements */
    GstElement *before_sink_queue = gst_element_factory_make("queue", "before-image-sink-queue");
    GstElement *converter = gst_element_factory_make("nvvideoconvert", "image-sink-video-converter");
    GstElement *caps_filter = gst_element_factory_make("capsfilter", "image-sink-caps-filter");
    GstElement *encoder = gst_element_factory_make("jpegenc", "image-sink-jpeg-encoder");
    GstElement *file_sink = gst_element_factory_make("multifilesink", "image-multi-file-sink");

    if(!before_sink_queue || !converter || !caps_filter || !encoder || !file_sink){
        g_printerr("One of the elements of image-sink %d could not be created. Exiting!\n", sink_number);
        return NULL;
    }

    /* Configure image-sink to create 1 min long files and save it to proper directory */
    gchar output_directory[MAX_DIRECTORY_PATH_SIZE];
    gchar output_file_format[MAX_DIRECTORY_PATH_SIZE];
    g_snprintf(output_directory, MAX_DIRECTORY_PATH_SIZE,
               "%s/%s", FILE_SINK_DIRECTORY, sink_bin_name);
    g_snprintf(output_file_format, MAX_DIRECTORY_PATH_SIZE,
               "%s/%s", output_directory, JPEG_NAME_FORMAT);

    /* Create directory and delete all files in it */
    struct stat st = {0};
    if (stat(output_directory, &st) == -1) {
        mkdir(output_directory, 0777);
    }
    delete_all_files_in_directory(output_directory);

    g_object_set(G_OBJECT(file_sink),
                 "location", output_file_format,
                 NULL);
    GstCaps *caps = gst_caps_new_simple(
            "video/x-raw",
            "format", G_TYPE_STRING, "I420",
            "width", G_TYPE_INT, JPEG_WIDTH,
            "height", G_TYPE_INT, JPEG_HEIGHT,
            NULL);
    g_object_set (G_OBJECT (caps_filter), "caps", caps, NULL);

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                     before_sink_queue,
                     converter,
                     caps_filter,
                     encoder,
                     file_sink,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(before_sink_queue,
                               converter,
                               caps_filter,
                               encoder,
                               file_sink,
                               NULL)) {
        g_printerr("Cannot link elements of %d image-sink-bin. Exiting.\n", sink_number);
        return NULL;
    }

    add_ghost_sink_pad_to_bin(bin, before_sink_queue, "sink");

    return bin;
}


GstElement *create_fake_sink_bin(guint sink_number){
    GstElement *bin = NULL;
    /* Create a sink GstBin to abstract this bin's content from the rest of the pipeline */
    gchar sink_bin_name[BIN_NAME_LENGTH];
    g_snprintf(sink_bin_name, BIN_NAME_LENGTH, "fake-sink-bin-%d", sink_number);
    bin = gst_bin_new(sink_bin_name);

    /* Create all file-sink elements */
    GstElement *before_sink_queue = gst_element_factory_make("queue", "before-fake-sink-queue");
    GstElement *fake_sink = gst_element_factory_make("fakesink", "fake-sink");

    if(!before_sink_queue || !fake_sink){
        g_printerr("One of the elements of fake-sink %d could not be created. Exiting!\n", sink_number);
        return NULL;
    }

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(bin),
                     before_sink_queue,
                     fake_sink,
                     NULL);

    /* Link all elements together */
    if (!gst_element_link_many(before_sink_queue,
                               fake_sink,
                               NULL)) {
        g_printerr("Cannot link elements of %d fake-sink-bin. Exiting.\n", sink_number);
        return NULL;
    }

    add_ghost_sink_pad_to_bin(bin, before_sink_queue, "sink");

    return bin;
}