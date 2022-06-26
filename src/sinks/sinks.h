//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_SINKS_H
#define KITTEN_DETECTOR_SINKS_H

#include <gst/gst.h>
#include <glib.h>

#define MAX_DIRECTORY_PATH_SIZE 1024
#define FILE_SINK_DIRECTORY "/output"
#define FILE_NAME_FORMAT "video-%02d.mp4"

GstElement *create_tilled_display_sink_bin(guint sink_bin_number, guint sinks_number);
GstElement *create_file_sink_bin(guint sink_number);

#endif //KITTEN_DETECTOR_SINKS_H
