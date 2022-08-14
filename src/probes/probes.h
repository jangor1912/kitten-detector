//
// Created by Jan Gorazda on 12/08/2022.
//

#ifndef KITTEN_DETECTOR_PROBES_H
#define KITTEN_DETECTOR_PROBES_H

#include <gst/gst.h>
#include <glib.h>

#include "structures/structures.h"

GstPadProbeReturn stop_recording_probe_callback(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data);

GstPadProbeReturn start_recording_probe_callback(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data);
GstPadProbeReturn attach_common_meta_data_to_buffer_probe(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data);

#endif //KITTEN_DETECTOR_PROBES_H
