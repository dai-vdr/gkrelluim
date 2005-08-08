/*
  uim-helper.c - part of uim for GKrellUIM 

  Copyright (c) 2003-2005 uim Project http://uim.freedesktop.org/
  Copyright (c) 2004-2005, dai <d+gkrelluim@vdr.jp>
  All rights reserved.

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

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

/* GTK+ */
#include <gtk/gtk.h>

/* UIM */
#include <uim/uim.h>
#include <uim/uim-helper.h>
#define OBJECT_DATA_SIZE_GROUP "SIZE_GROUP"
static unsigned int read_tag;
/* GKrellUIM: static */ int uim_fd;

/* GKrellUIM */
extern gchar *mode_text;
extern gchar *input_text;

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 */
static gchar *
get_charset(gchar *line)
{
  gchar **splitted = NULL;

  splitted = g_strsplit(line, "=", 0);

  if(splitted && splitted[0] && splitted[1]
     && strcmp("charset", splitted[0]) == 0) {
    gchar *charset = g_strdup(splitted[1]);
    g_strfreev(splitted);
    return charset;
  } else {
    g_strfreev(splitted);
    return NULL;
  }
}

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 */
static gchar *
convert_charset(const gchar *charset, const gchar *line)
{
  if(charset == NULL) {
    return NULL;
  }
  return g_convert(line, strlen(line),
                   "UTF-8", charset, 
                   NULL, /* gsize *bytes_read */
                   NULL, /*size *bytes_written */
                   NULL); /* GError **error*/
}

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_list_update(GtkWidget *widget, gchar **tmp)
{
  int i = 0;
  gchar **tmp2 = NULL;
  gchar *charset = NULL;
  /* GKrellUIM */
  gint branch_number = 0;

  charset = get_charset(tmp[1]);

  /* XXX: remove menu_buttons */

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );
  input_text = g_strdup( "-" );

  while(tmp[i] && strcmp("", tmp[i]) != 0) {
    gchar *utf8_str;
    utf8_str = convert_charset(charset, tmp[i]);

    if(utf8_str != NULL) {
      tmp2 = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      tmp2 = g_strsplit(tmp[i], "\t", 0);
    }

    if(tmp2 && tmp2[0])
    {
      if(strcmp("branch", tmp2[0]) == 0) {
        /* XXX: remove menu_buttons */
        /* GKrellUIM */
        branch_number++;
        if( i == 2 ) {
          mode_text = g_strdup( tmp2[1] );
        }

        /* GKrellUIM */
        if( branch_number == 2 ) {
          input_text = g_strdup( tmp2[1] );
        }
      } /* else if(strcmp("leaf", tmp2[0]) == 0) {
           XXX: remove menu_buttons
      } */
      g_strfreev(tmp2);
    }
    i++;
  }

  /* XXX: remove menu_buttons */

  if(charset)
    g_free(charset);
}

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_label_update(GtkWidget *widget, gchar **lines)
{
  unsigned int i = 0;
  gchar **pair = NULL;
  gchar *charset = NULL;

  charset = get_charset(lines[1]);

  while(lines[i] && strcmp("", lines[i]) != 0) {
    i++;
  }

  /* XXX: remove menu_buttons */

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );

  i = 1; /* resetting temporary variable */

  while(lines[i] && strcmp("", lines[i]) != 0) {

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
   
    if(pair && pair[0] && pair[1])
    {
      /* XXX: remove menu_buttons */
      /* GKrellUIM */
      if( i - 2 > 0 ) {
        ;
      } else {
        /* GKrellUIM */
        mode_text = g_strdup( pair[0] );
      }
    }
    g_strfreev(pair);
    i++;
  }

  if(charset)
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
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 */
static void
helper_toolbar_parse_helper_str(GtkWidget *widget, gchar *str)
{
  gchar **lines;
  lines = g_strsplit(str, "\n", 0);

  if(lines && lines[0]) {
    if( strcmp("prop_list_update", lines[0]) == 0) {
      helper_toolbar_prop_list_update(widget, lines);
    } else if( strcmp("prop_label_update", lines[0]) == 0) {
      helper_toolbar_prop_label_update(widget, lines);
    }
    g_strfreev(lines);
  }
}

/*
 * taken from uim-0.4.7alpha1/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static gboolean
fd_read_cb(GIOChannel *channel, GIOCondition c, gpointer p)
{
  gchar *tmp;
  int fd = g_io_channel_unix_get_fd(channel);
  GtkWidget *widget = GTK_WIDGET(p);

  uim_helper_read_proc(fd);

  /* XXX: removed helper_icon_parse_helper_str */

  while ((tmp = uim_helper_get_message())) {
    helper_toolbar_parse_helper_str(widget, tmp);
    /*fprintf( stderr, "DEBUG>> %s\n", tmp );*/
    free(tmp); tmp = NULL;
  }

  return TRUE;
}

/*
 * taken from uim-0.4.6beta2/helper/toolbar-common-gtk.c
 */
/* GKrellUIM: static */ void
check_helper_connection(GtkWidget *widget)
{
  if(uim_fd < 0) {
    uim_fd = uim_helper_init_client_fd(helper_disconnect_cb);
    if(uim_fd > 0) {
      GIOChannel *channel;
      channel = g_io_channel_unix_new(uim_fd);
      read_tag = g_io_add_watch(channel, G_IO_IN | G_IO_HUP | G_IO_ERR,
                                fd_read_cb, (gpointer)widget);
      g_io_channel_unref(channel);
    }
  }
}
