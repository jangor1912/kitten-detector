//
// Created by jan on 15.04.22.
//
// Stolen from deepstream-sample-apps/deepstream-nvof-test/deepstream_nvof_test.c
#include <gst/gst.h>
#include <glib.h>

#include "sources.h"
#include "decoders.h"
#include "utils/utils.h"

/* NVIDIA Decoder source pad memory feature. This feature signifies that source
 * pads having this capability will push GstBuffers containing cuda buffers. */
#define GST_CAPS_FEATURES_NVMM "memory:NVMM"


/* PRIVATE FUNCTIONS */
static void cb_newpad(GstElement *decodebin, GstPad *decoder_src_pad, gpointer data);
static void decodebin_child_added(GstChildProxy *child_proxy, GObject *object,
                                  gchar *name, gpointer user_data);

/* PUBLIC FUNCTIONS */
GstElement *create_source_bin(guint source_number, gchar *source_uri, gchar* codec_string){
    enum VideoCodec codec = codec_from_string(codec_string);

    if(codec == UNSUPPORTED) {
        g_print("Codec is unsupported - falling back to uridecodebin - this may cause some issues!\n");
        return create_uridecode_source_bin(source_number, source_uri);
    } else {
        g_printerr("Currently codec-specific source-bins are not supported!\n");
        return NULL;
    }
}

GstElement *create_uridecode_source_bin(guint source_number, gchar *source_uri) {
    GstElement *bin = NULL, *uri_decode_bin = NULL;
    gchar bin_name[BIN_NAME_LENGTH] = {};

    g_snprintf(bin_name, BIN_NAME_LENGTH, "uridecode-source-bin-%02d", source_number);
    /* Create a source GstBin to abstract this bin's content from the rest of the pipeline */
    bin = gst_bin_new(bin_name);

    /* Source element for reading from the uri.
     * We will use decodebin and let it figure out the container format of the
     * stream and the codec and plug the appropriate demux and decode plugins. */
    uri_decode_bin = gst_element_factory_make("uridecodebin", "uri-decode-bin");

    if (!bin || !uri_decode_bin) {
        g_printerr("One element in source bin could not be created.\n");
        return NULL;
    }

    /* We set the input uri to the source element */
    g_object_set(G_OBJECT(uri_decode_bin), "uri", source_uri, NULL);

    /* Connect to the "pad-added" signal of the decodebin which generates a
     * callback once a new pad for raw data has been created by the decodebin */
    g_signal_connect(G_OBJECT(uri_decode_bin), "pad-added",
                     G_CALLBACK(cb_newpad), bin);
    g_signal_connect(G_OBJECT(uri_decode_bin), "child-added",
                     G_CALLBACK(decodebin_child_added), bin);

    gst_bin_add(GST_BIN(bin), uri_decode_bin);

    /* We need to create a ghost pad for the source bin which will act as a proxy
     * for the video decoder src pad. The ghost pad will not have a target right
     * now. Once the decode-bin creates the video decoder and generates the
     * cb_newpad callback, we will set the ghost pad target to the video decoder
     * src pad. */
    if (!gst_element_add_pad(bin, gst_ghost_pad_new_no_target("src",
                                                              GST_PAD_SRC))) {
        g_printerr("Failed to add ghost pad in source bin\n");
        return NULL;
    }

    return bin;
}

/* IMPLEMENTATION OF PRIVATE FUNCTIONS */

static void cb_newpad(GstElement *decodebin, GstPad *decoder_src_pad, gpointer data) {
    g_print("In cb_newpad\n");
    GstCaps *caps = gst_pad_get_current_caps(decoder_src_pad);
    const GstStructure *str = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(str);
    GstElement *source_bin = (GstElement *) data;
    GstCapsFeatures *features = gst_caps_get_features(caps, 0);

    /* Need to check if the pad created by the decodebin is for video and not audio. */
    if (!strncmp(name, "video", 5)) {
        /* Link the decodebin pad only if decodebin has picked nvidia
         * decoder plugin nvdec_*. We do this by checking if the pad caps contain
         * NVMM memory features. */
        if (gst_caps_features_contains(features, GST_CAPS_FEATURES_NVMM)) {
            /* Get the source bin ghost pad */
            GstPad *bin_ghost_pad = gst_element_get_static_pad(source_bin, "src");
            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(bin_ghost_pad),
                                          decoder_src_pad)) {
                g_printerr("Failed to link decoder src pad to source bin ghost pad\n");
            }
            gst_object_unref(bin_ghost_pad);
        } else {
            g_printerr("Error: Decodebin did not pick nvidia decoder plugin.\n");
        }
    }
}

static void decodebin_child_added(GstChildProxy *child_proxy, GObject *object,
                                  gchar *name, gpointer user_data) {
    g_print("Decodebin child added: %s\n", name);
    if (g_strrstr(name, "decodebin") == name) {
        g_signal_connect(G_OBJECT(object), "child-added",
                         G_CALLBACK(decodebin_child_added), user_data);
    }
}
