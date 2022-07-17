//
// Created by Jan Gorazda on 21/05/2022.
//
#include <gst/gst.h>
#include <glib.h>
#include <gst/base/gstbaseparse.h>
#include <stdio.h>
#include <dirent.h>

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
    write_pad_name(final_sink_pad_name, PAD_NAME_LENGTH,
                   sink_pad_name, "sink");
    
    GstPad *pad = gst_element_get_static_pad(bin_first_element, final_sink_pad_name);
    if(!pad){
        g_printerr("Cannot retrieve static-pad (%s) from first-bin element!\n", final_sink_pad_name);
        return;
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
        g_printerr("Cannot retrieve static-pad (%s) from last-bin element!\n", final_src_pad_name);
        return;
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

int delete_all_files_in_directory(char *directory_path){
    // These are data types defined in the "dirent" header
    DIR *theFolder = opendir(directory_path);
    struct dirent *next_file;
    char filepath[MAX_DIRECTORY_PATH_SIZE];

    while ( (next_file = readdir(theFolder)) != NULL )
    {
        // build the path for each file in the folder
        g_snprintf(filepath, MAX_DIRECTORY_PATH_SIZE,
                   "%s/%s", directory_path, next_file->d_name);
        remove(filepath);
    }
    closedir(theFolder);
    return 0;
}

void solve_lacking_pts_timestamps(GstBin* bin){
    // workaround no PTS issue
    // TODO does not work - iterator next results in Segmentation Fault
    // https://gist.github.com/zougloub/0747f84d45bc35413c0c19584c398b3d
    // https://stackoverflow.com/questions/42874691/gstreamer-for-android-buffer-has-no-pts
    GstIterator *itr = NULL;
    GValue item;
    gpointer ptr;
    gboolean done = FALSE;
    GstElement *element = NULL;
    gchar *element_name;
    GstIteratorResult iterator_result;
    g_print("Before extracting iterator!\n");
    itr = gst_bin_iterate_elements(bin);
    g_print("After extracting iterator!\n");

    while(!done){
        g_print("Before iterator next!\n");
        iterator_result = gst_iterator_next (itr, &item);
        g_print("After iterator next!\n");
        switch (iterator_result) {
            case GST_ITERATOR_OK:
                g_print("In iterator GST_ITERATOR_OK!\n");
                ptr = g_value_peek_pointer(&item);
                g_print("In iterator after pointer extraction!\n");
                element = GST_ELEMENT(ptr);
                g_print("In iterator after element casting!\n");
                element_name = gst_element_get_name(element);
                g_print("In iterator after getting element name which is %s!\n", element_name);
                if(strcmp(element_name, "parse")){
                    g_print("In iterator Element is 'parse'!\n");
                    gst_base_parse_set_infer_ts(GST_BASE_PARSE(element), TRUE);
                    g_print("In iterator after gst_base_parse_set_infer_ts!\n");
                    gst_base_parse_set_pts_interpolation(GST_BASE_PARSE(element), TRUE);
                    g_print("In iterator after gst_base_parse_set_pts_interpolation!\n");
                }
                break;
            default:
                g_print("In iterator default!\n");
                done = TRUE;
                break;
        }
    }
    gst_iterator_free (itr);
}
