//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_SINKS_H
#define KITTEN_DETECTOR_SINKS_H

#include <gst/gst.h>
#include <glib.h>

GstElement *create_tilled_display_sink_bin(gint sink_bin_number, gint sinks_number);

#endif //KITTEN_DETECTOR_SINKS_H
