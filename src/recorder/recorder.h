//
// Created by Jan Gorazda on 31/01/2023.
//

#ifndef KITTEN_DETECTOR_RECORDER_H
#define KITTEN_DETECTOR_RECORDER_H

#include <gst/gst.h>
#include <glib.h>

#include "structures/structures.h"

GstElement *create_recorder_bin(guint sink_number);
void stop_recording_handler(GstElement *src, GstPad *new_pad, gpointer recorder_data);
void start_recording_handler(GstElement *src, GstPad *new_pad, gpointer recorder_data);
void switch_recorder_state_handler(GstElement *src, GstPad *new_pad, gpointer recorder_data);

#endif //KITTEN_DETECTOR_RECORDER_H
