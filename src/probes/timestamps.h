//
// Created by jan on 07.11.22.
//

#ifndef KITTEN_DETECTOR_TIMESTAMPS_H
#define KITTEN_DETECTOR_TIMESTAMPS_H

#include <gst/gst.h>
#include <glib.h>

#include "metadata/metadata.h"

GstPadProbeReturn add_pre_decoder_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
GstPadProbeReturn add_post_decoder_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
GstPadProbeReturn add_pre_inference_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
GstPadProbeReturn add_post_inference_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
GstPadProbeReturn add_pre_tracker_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
GstPadProbeReturn add_post_tracker_timestamp_to_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

#endif //KITTEN_DETECTOR_TIMESTAMPS_H
