//
// Created by Jan Gorazda on 21/05/2022.
//

#ifndef KITTEN_DETECTOR_UTILS_H
#define KITTEN_DETECTOR_UTILS_H

#include <gst/gst.h>
#include <glib.h>

#include <stdint.h>
#include <time.h>

#define BIN_NAME_LENGTH 128
#define ELEMENT_NAME_LENGTH 128
#define PAD_NAME_LENGTH 128
#define MAX_DIRECTORY_PATH_SIZE 1024
#define OK 0
#define FAIL 1

void add_ghost_sink_pad_to_bin(GstElement *bin, GstElement *bin_first_element, gchar *sink_pad_name);
void add_ghost_src_pad_to_bin(GstElement *bin, GstElement *bin_last_element, gchar *src__pad_name);
gint connect_two_elements(
        GstElement *first_element, GstElement *second_element,
        gchar *sink_pad_name, gchar* src_pad_name
);
int delete_all_files_in_directory(char *directory_path);
void solve_lacking_pts_timestamps(GstBin* bin);

#endif //KITTEN_DETECTOR_UTILS_H
