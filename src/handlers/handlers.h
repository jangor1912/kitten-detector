//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_HANDLERS_H
#define KITTEN_DETECTOR_HANDLERS_H

#include <gst/gst.h>
#include <glib.h>

gboolean bus_call (GstBus * bus, GstMessage * msg, gpointer data);

#endif //KITTEN_DETECTOR_HANDLERS_H
