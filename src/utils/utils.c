//
// Created by Jan Gorazda on 21/05/2022.
//
#include <gst/gst.h>
#include <glib.h>

#include "utils.h"

void write_pad_name(gchar *buffer, size_t buffer_size, gchar *pad_name, gchar *default_pad_name){
    if(!pad_name){
        g_snprintf(buffer, buffer_size, "%s", default_pad_name);
    } else {
        g_snprintf(buffer, buffer_size, "%s", pad_name);
    }
}

void add_ghost_sink_pad_to_bin(GstElement *bin, GstElement *bin_first_element, gchar *sink_pad_name) {
    gchar final_sink_pad_name[PAD_NAME_LENGTH];
    write_pad_name(sink_pad_name, PAD_NAME_LENGTH,
                   sink_pad_name, "sink");
    
    GstPad *pad = gst_element_get_static_pad(bin_first_element, final_sink_pad_name);
    if(!pad){
        pad = gst_element_get_request_pad(bin_first_element, final_sink_pad_name);
    }
    GstPad *ghost_pad = gst_ghost_pad_new(final_sink_pad_name, pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(bin, ghost_pad);
    gst_object_unref(pad);
}

void add_ghost_src_pad_to_bin(GstElement *bin, GstElement *bin_last_element, gchar *src_pad_name) {
    gchar final_src_pad_name[PAD_NAME_LENGTH];
    write_pad_name(final_src_pad_name, PAD_NAME_LENGTH,
                   src_pad_name, "src");
    
    GstPad *pad = gst_element_get_static_pad(bin_last_element, final_src_pad_name);
    if(!pad){
        pad = gst_element_get_request_pad(bin_last_element, final_src_pad_name);
    }
    GstPad *ghost_pad = gst_ghost_pad_new(final_src_pad_name, pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(bin, ghost_pad);
    gst_object_unref(pad);
}

gint connect_two_elements(
        GstElement *first_element, GstElement *second_element,
        gchar *sink_pad_name, gchar* src_pad_name
){
    gchar final_sink_pad_name[PAD_NAME_LENGTH];
    gchar final_src_pad_name[PAD_NAME_LENGTH];

    write_pad_name(final_sink_pad_name, PAD_NAME_LENGTH,
                   sink_pad_name, "sink");
    write_pad_name(final_src_pad_name, PAD_NAME_LENGTH,
                   src_pad_name, "src");


    GstPad *first_element_src_pad = gst_element_get_static_pad(first_element, final_src_pad_name);
    if (!first_element_src_pad) {
        first_element_src_pad = gst_element_get_request_pad(first_element, final_src_pad_name);
        if(!first_element_src_pad) {
            g_printerr("Cannot retrieve src_pad (%s) of first element.\n", final_src_pad_name);
            return FAIL;
        }
    }

    GstPad *second_element_sink_pad = gst_element_get_static_pad(second_element, final_sink_pad_name);
    if (!second_element_sink_pad) {
        second_element_sink_pad = gst_element_get_request_pad(second_element, final_sink_pad_name);
        if(!second_element_sink_pad){
            g_printerr("Cannot get sink-pad (%s) of second-element!\n", final_sink_pad_name);
            return FAIL;
        }
    }

    if (gst_pad_link(first_element_src_pad, second_element_sink_pad) != GST_PAD_LINK_OK) {
        g_printerr("Cannot link two elements together!\n");
        return FAIL;
    }
    gst_object_unref(first_element_src_pad);
    gst_object_unref(second_element_sink_pad);

    return OK;
}
