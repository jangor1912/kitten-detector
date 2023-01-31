//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_SINKS_H
#define KITTEN_DETECTOR_SINKS_H

#include <gst/gst.h>
#include <glib.h>

#define FILE_SINK_DIRECTORY "/project/output"
#define FILE_NAME_FORMAT "video-%02d.mp4"
#define JPEG_NAME_FORMAT "%05d.jpg"

#define JPEG_WIDTH 3840
#define JPEG_HEIGHT 2160

GstElement *create_tilled_display_sink_bin(guint sink_bin_number, guint sinks_number);
GstElement *create_file_sink_bin(guint sink_number);
GstElement *create_image_sink_bin(guint sink_number);
GstElement *create_fake_sink_bin(guint sink_number);

#endif //KITTEN_DETECTOR_SINKS_H
