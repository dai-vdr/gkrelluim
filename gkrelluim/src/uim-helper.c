/*
  GKrellUIM uim-helper 

  Copyright (C) 2004-2006 dai <d+gkrelluim@vdr.jp>
  All rights reserved.

  Original Author: uim Project http://uim.freedesktop.org/
  Modifier: dai <d+gkrelluim@vdr.jp>

  Redistribution and use in source and binary forms, with or
  without modification, are permitted provided that the
  following conditions are met:

  1. Redistributions of source code must retain the above
     copyright notice, this list of conditions and the
     following disclaimer.
  2. Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the
     following disclaimer in the documentation and/or other
     materials provided with the distribution.
  3. Neither the name of authors nor the names of its
     contributors may be used to endorse or promote products
     derived from this software without specific prior written
     permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* GTK+ */
#include <gtk/gtk.h>

/* UIM */
#include <uim/uim.h>
#include <uim/uim-helper.h>
#include <string.h>
#include <stdlib.h>
#define OBJECT_DATA_SIZE_GROUP "SIZE_GROUP"
static unsigned int read_tag;
/* GKrellUIM: static */ int uim_fd;

/* GKrellUIM */
extern gchar *mode_text;
extern gchar *input_text;

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static gchar *
get_charset(gchar *line)
{
  gchar **tokens;
  gchar *charset = NULL;

  tokens = g_strsplit(line, "=", 0);
  if (tokens && tokens[0] && tokens[1] && !strcmp("charset", tokens[0]))
    charset = g_strdup(tokens[1]);
  g_strfreev(tokens);

  return charset;
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static gchar *
convert_charset(const gchar *charset, const gchar *str)       
{
  if (!charset)
    return NULL;

  return g_convert(str, strlen(str),
                   "UTF-8", charset,
                   NULL, /* gsize *bytes_read */
                   NULL, /* size *bytes_written */
                   NULL); /* GError **error */
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_list_update(GtkWidget *widget, gchar **lines)
{
  /* XXX: remove button */

  int i;
  gchar **cols;
  gchar *charset = NULL;
  /* GKrellUIM */
  gint branch_number = 0;

  /* XXX: remove prop_buttons & tool_buttons */

  charset = get_charset(lines[1]);

  /* XXX: remove prop_buttons & tool_buttons */

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );
  input_text = g_strdup( "-" );

  i = 0;
  while (lines[i] && strcmp("", lines[i])) {
    gchar *utf8_str = convert_charset(charset, lines[i]);

    if (utf8_str != NULL) {
      cols = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      cols = g_strsplit(lines[i], "\t", 0);
    }

    if (cols && cols[0]) {
      if (!strcmp("branch", cols[0])) {
        /* XXX: remove button */
        /* GKrellUIM */
        branch_number++;
        if( i == 2 ) {
          mode_text = g_strdup( cols[1] );
        }
        if( branch_number == 2 ) {
          input_text = g_strdup( cols[1] );
        }
      } /* else if (!strcmp("leaf", cols[0])) {
           XXX: remove button
      } */
      g_strfreev(cols);
    }
    i++;
  }

  /* XXX: remove tool_button */

  g_free(charset);
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_label_update(GtkWidget *widget, gchar **lines)
{
  /* XXX: remove button */

  unsigned int i;
  gchar **pair;
  gchar *charset = NULL;

  /* XXX: remove prop_buttons */

  i = 0;
  while (lines[i] && strcmp("", lines[i]))
    i++;                                                        

  /* XXX: remove prop_buttons */

  charset = get_charset(lines[1]);

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );

  i = 2;
  while (lines[i] && strcmp("", lines[i])) {
    if (charset) {
      gchar *utf8_str;
      utf8_str = g_convert(lines[i], strlen(lines[i]),
                           "UTF-8", charset,
                           NULL, /* gsize *bytes_read */
                           NULL, /*size *bytes_written */
                           NULL); /* GError **error*/
      pair = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      pair = g_strsplit(lines[i], "\t", 0);
    }

    if (pair && pair[0] && pair[1]) {
      /* XXX: remove prop_buttons */
      /* GKrellUIM */
      if( i - 2 > 0 ) {
        ;
      } else {
        mode_text = g_strdup( pair[0] );
      }
    }
    g_strfreev(pair);
    i++;
  }

  g_free(charset);
}

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 */
static void
helper_disconnect_cb(void)
{
  uim_fd = -1;
  g_source_remove(read_tag);
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
void
uim_toolbar_get_im_list(void)
{
  uim_helper_send_message(uim_fd, "im_list_get\n");
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static void
helper_toolbar_im_list_update(GtkWidget *widget, gchar **lines)
{
  /* XXX: remove im_button */

  int i;
  gchar **cols;
  gchar *charset = NULL;

  charset = get_charset(lines[1]);

  /* XXX: remove im_button */

  i = 2;
  while (lines[i] && strcmp("", lines[i])) {
    gchar *utf8_str = convert_charset(charset, lines[i]);

    if (utf8_str != NULL) {
      cols = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      cols = g_strsplit(lines[i], "\t", 0);
    }
    if (cols && cols[0]) {
      /* XXX: remove im_button */
      g_strfreev(cols);
    }
    i++;
  }
  g_free(charset);
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static void
helper_toolbar_parse_helper_str(GtkWidget *widget, gchar *str)
{
  gchar **lines;
  lines = g_strsplit(str, "\n", 0);

  if (lines && lines[0]) {
    if (!strcmp("prop_list_update", lines[0]))
      helper_toolbar_prop_list_update(widget, lines);
    else if (!strcmp("prop_label_update", lines[0]))
      helper_toolbar_prop_label_update(widget, lines);
    else if (!strcmp("focus_in", lines[0]))
      uim_toolbar_get_im_list();
    else if (g_str_has_prefix(lines[0], "im_list"))
      helper_toolbar_im_list_update(widget, lines);
    /* else if (!strcmp("custom_reload_notify", lines[0])) {
      uim_prop_reload_configs();
      helper_toolbar_check_custom();
    } */
    g_strfreev(lines);
  }
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static gboolean
fd_read_cb(GIOChannel *channel, GIOCondition c, gpointer p)
{
  gchar *msg;
  int fd = g_io_channel_unix_get_fd(channel);
  GtkWidget *widget = GTK_WIDGET(p);

  uim_helper_read_proc(fd);

  while ((msg = uim_helper_get_message())) {
    helper_toolbar_parse_helper_str(widget, msg);
    /*fprintf(stderr, "DEBUG>> %s\n", msg);*/
    free(msg);
  }

  return TRUE;
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
void
uim_toolbar_check_helper_connection(GtkWidget *widget)
{
  if (uim_fd < 0) {
    uim_fd = uim_helper_init_client_fd(helper_disconnect_cb);
    if (uim_fd > 0) {
      GIOChannel *channel;
      channel = g_io_channel_unix_new(uim_fd);
      read_tag = g_io_add_watch(channel, G_IO_IN | G_IO_HUP | G_IO_ERR,
                                fd_read_cb, (gpointer)widget);
      g_io_channel_unref(channel);
    }
  }
}
