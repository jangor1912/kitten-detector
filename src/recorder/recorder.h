//
// Created by Jan Gorazda on 31/01/2023.
//

#ifndef KITTEN_DETECTOR_RECORDER_H
#define KITTEN_DETECTOR_RECORDER_H

#include <gst/gst.h>
#include <glib.h>

GstElement *create_recorder_bin(guint sink_number);

#endif //KITTEN_DETECTOR_RECORDER_H
