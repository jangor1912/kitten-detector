//
// Created by Jan Gorazda on 12/08/2022.
//

#ifndef KITTEN_DETECTOR_PROBES_H
#define KITTEN_DETECTOR_PROBES_H

#include <gst/gst.h>
#include <glib.h>

#include "structures/structures.h"
#include "metadata/metadata.h"

typedef void (*common_metadata_handler_t)(CommonMetaData *, gpointer u_data);

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

GstPadProbeReturn on_new_buffer_with_common_metadata(
        GstPad *pad,
        GstPadProbeInfo *info,
        gpointer u_data,
        common_metadata_handler_t common_metadata_handler);

GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad * pad, GstPadProbeInfo * info, gpointer u_data);
GstPadProbeReturn recorder_manager_buffer_probe(GstPad * pad, GstPadProbeInfo * info, gpointer u_data);

#endif //KITTEN_DETECTOR_PROBES_H
