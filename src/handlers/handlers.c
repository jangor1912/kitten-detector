//
// Created by jan on 15.04.22.
//
#include <gst/gst.h>
#include <glib.h>

#include "handlers.h"

gboolean bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_WARNING:
        {
            gchar *debug;
            GError *error;
            gst_message_parse_warning (msg, &error, &debug);
            g_printerr ("WARNING from element %s: %s\n",
                        GST_OBJECT_NAME (msg->src), error->message);
            g_free (debug);
            g_printerr ("Warning: %s\n", error->message);
            g_error_free (error);
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *error;
            gst_message_parse_error (msg, &error, &debug);
            g_printerr ("ERROR from element %s: %s\n",
                        GST_OBJECT_NAME (msg->src), error->message);
            if (debug)
                g_printerr ("Error details: %s\n", debug);
            g_free (debug);
            g_error_free (error);
            g_main_loop_quit (loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}