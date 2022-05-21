//
// Created by jan on 15.04.22.
//

#ifndef KITTEN_DETECTOR_SOURCES_H
#define KITTEN_DETECTOR_SOURCES_H

#include <gst/gst.h>
#include <glib.h>

typedef struct {
    guint sources_number;
    gchar **source_uris;

} SourcesConfig;

GstElement *create_uridecode_source_bin(guint source_number, gchar *source_uri);

#endif //KITTEN_DETECTOR_SOURCES_H
