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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <uim/uim.h>
#include <uim/uim-helper.h>
static unsigned int read_tag;
/* GKrellUIM: static */ int uim_fd;
static GtkIconFactory *uim_factory;
static GList *uim_icon_list;

static gboolean register_icon(const gchar *name);

/* GKrellUIM */
#include <gkrellm2/gkrellm.h>
extern gchar *mode_text;
extern gchar *input_text;

const gchar *prop_icon[]    = { "im_icon",    "mode_icon",    "input_icon"    };
const gchar *prop_label[]   = { "im_label",   "mode_label",   "input_label"   };
const gchar *prop_tooltip[] = { "im_tooltip", "mode_tooltip", "input_tooltip" };
const gchar *prop_action[]  = { "im_action",  "mode_action",  "input_action"  };
const gchar *prop_state[]   = { "im_state",   "mode_state",   "input_state"   };

enum {
  TYPE_IM=0,
  TYPE_MODE=1,
  TYPE_INPUT=2
};

void uim_toolbar_check_helper_connection(GtkWidget*);

/*
 * taken from uim-svn3109/helper/toolbar-common-gtk.c
 */
struct _CommandEntry {
  const gchar *desc;
  const gchar *label;
  const gchar *icon;
  const gchar *command;
  const gchar *custom_button_show_symbol;
  uim_bool show_button;
};

/*
 * taken from uim-svn3109/helper/toolbar-common-gtk.c
 */
/* FIXME! command menu and buttons should be customizable. */
static struct _CommandEntry command_entry[] = {
  {
    N_("Switch input method"),
    NULL,
    "switcher-icon",
    "uim-im-switcher-gtk &",
    "toolbar-show-switcher-button?",
    UIM_FALSE
  },

  {
    N_("Preference"),
    NULL,
    GTK_STOCK_PREFERENCES,
    "uim-pref-gtk &",
    "toolbar-show-pref-button?",
    UIM_FALSE
  },

  {
    N_("Japanese dictionary editor"),
    "Dic",
    NULL,
    "uim-dict-gtk &",
    "toolbar-show-dict-button?",
    UIM_FALSE
  },

  {
    N_("Input pad"),
    "Pad",
    NULL,
    "uim-input-pad-ja &",
    "toolbar-show-input-pad-button?",
    UIM_FALSE
  },

  {
    N_("Handwriting input pad"),
    "Hand",
    NULL,
    "uim-tomoe-gtk &",
    "toolbar-show-handwriting-input-pad-button?",
    UIM_FALSE
  },

  {
    N_("Help"),
    NULL,
    GTK_STOCK_HELP,
    "uim-help &",
    "toolbar-show-help-button?",
    UIM_FALSE
  }
};

/* GKrellUIM: static */ guint command_entry_len = sizeof(command_entry) / sizeof(struct _CommandEntry);

/* GKrellUIM */
const gchar *
get_command_entry_desc( gint data ) {
  return command_entry[ data ].desc;
}

/* GKrellUIM */
const gchar *
get_command_entry_command( gint data ) {
  return command_entry[ data ].command;
}

/* GKrellUIM */
const gchar *
get_command_entry_custom_button_show_symbol( gint data ) {
  return command_entry[ data ].custom_button_show_symbol;
}

/*
 * taken from uim-svn3110/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_check_custom()
{
  guint i;

  /* GKrellUIM */
  uim_init();

  for (i = 0; i < command_entry_len; i++)
    command_entry[i].show_button =
      uim_scm_symbol_value_bool(command_entry[i].custom_button_show_symbol);
}

/*
 * taken from uim-svn3144/helper/toolbar-common-gtk.c
 */
static const char *
safe_gettext(const char *msgid)
{ 
  const char *p;

  for (p = msgid; *p && isascii(*p); p++)
    continue;

  return (*p) ? msgid : gettext(msgid);
}

/*
 * taken from uim-svn3144/helper/toolbar-common-gtk.c
 */
static gboolean
has_n_strs(gchar **str_list, guint n)
{
  guint i;

  if (!str_list)
    return FALSE;

  for (i = 0; i < n; i++) {
    if (!str_list[i])
      return FALSE;
  }

  return TRUE;
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static void
prop_menu_activate(GtkMenu *menu_item, gpointer data)
{
  GString *msg;

  msg = g_string_new((gchar *)g_object_get_data(G_OBJECT(menu_item),
                     "prop_action"));
  g_string_prepend(msg, "prop_activate\n");
  g_string_append(msg, "\n");
  uim_helper_send_message(uim_fd, msg->str);
  g_string_free(msg, TRUE);
}

/*
 * taken from uim-svn3485/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
/* GKrellUIM: popup_prop_menu */
create_prop_menu(GtkWidget *prop_menu, GtkWidget *widget, const gint type)
{
  GtkWidget *menu_item, *hbox, *label, *img;
  GtkTooltips *tooltip;
  GList *menu_item_list, *icon_list, *label_list, *tooltip_list, *action_list,
        *state_list /* GKrellUIM: *list */;
  int i, selected = -1;

  uim_toolbar_check_helper_connection(widget);

  menu_item_list = gtk_container_get_children(GTK_CONTAINER(prop_menu));
  /* GKrellUIM */
  icon_list    = g_object_get_data(G_OBJECT(widget), prop_icon   [type]);
  label_list   = g_object_get_data(G_OBJECT(widget), prop_label  [type]);
  tooltip_list = g_object_get_data(G_OBJECT(widget), prop_tooltip[type]);
  action_list  = g_object_get_data(G_OBJECT(widget), prop_action [type]);
  state_list   = g_object_get_data(G_OBJECT(widget), prop_state  [type]);

  /* XXX: remove gtk_widget_destroy */

  /* check selected item */
  i = 0;
  while (state_list) {
    if (!strcmp("*", state_list->data)) {
      selected = i;
      break;
    }
    state_list = state_list->next;
    i++;
  }

  i = 0;
  while (label_list) {
    if (selected != -1) {
      menu_item = gtk_check_menu_item_new();
      label = gtk_label_new(label_list->data);
      hbox = gtk_hbox_new(FALSE, 0);
#if GTK_CHECK_VERSION(2, 4, 0)
      gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(menu_item),
                                            TRUE);
#endif
      if (register_icon(icon_list->data))
        img = gtk_image_new_from_stock(icon_list->data, GTK_ICON_SIZE_MENU);
       else
        img = gtk_image_new_from_stock("null", GTK_ICON_SIZE_MENU);
      if (img) {
        gtk_box_pack_start(GTK_BOX(hbox), img, FALSE, FALSE, 3);
        gtk_widget_show(img);
      }
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 3);
      gtk_container_add(GTK_CONTAINER(menu_item), hbox);
      gtk_widget_show(label);
      gtk_widget_show(hbox);
      if (i == selected)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);
    } else {
      menu_item = gtk_menu_item_new_with_label(label_list->data);
    }

    /* tooltips */
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip(tooltip, menu_item,
                        tooltip_list ? tooltip_list->data : NULL, NULL);

    /* add to the menu */
    gtk_menu_shell_append(GTK_MENU_SHELL(prop_menu), menu_item);

    gtk_widget_show(menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate",
                     G_CALLBACK(prop_menu_activate), prop_menu);
    g_object_set_data(G_OBJECT(menu_item), "prop_action",
                     action_list? action_list->data : NULL);
    label_list = label_list->next;
    if (icon_list)
      icon_list = icon_list->next;
    if (action_list)
      action_list = action_list->next;
    if (tooltip_list)
      tooltip_list = tooltip_list->next;
    i++;
  }
}

/* GKrellUIM */
void
create_mode_menu( GtkWidget *menu, GtkWidget *event ) {
  create_prop_menu( menu, event, TYPE_MODE );
}

/* GKrellUIM */
void
create_input_menu( GtkWidget *menu, GtkWidget *event ) {
  create_prop_menu( menu, event, TYPE_INPUT );
}

/* GKrellUIM */
void
create_im_menu( GtkWidget *menu, GtkWidget *event ) {
  create_prop_menu( menu, event, TYPE_IM );
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static void
list_data_free(GList *list)
{
  g_list_foreach(list, (GFunc)g_free, NULL);
  g_list_free(list);
}

/*
 * taken from uim-svn3135/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
prop_data_flush(gpointer data, const gint type)
{
  GList *list;
  /* GKrellUIM */
  list = g_object_get_data(data, prop_icon   [type]);
  list_data_free(list);
  list = g_object_get_data(data, prop_label  [type]);
  list_data_free(list);
  list = g_object_get_data(data, prop_tooltip[type]);
  list_data_free(list);
  list = g_object_get_data(data, prop_action [type]);
  list_data_free(list);
  list = g_object_get_data(data, prop_state  [type]);
  list_data_free(list);

  /* GKrellUIM */
  g_object_set_data(G_OBJECT(data), prop_icon   [type], NULL);
  g_object_set_data(G_OBJECT(data), prop_label  [type], NULL);
  g_object_set_data(G_OBJECT(data), prop_tooltip[type], NULL);
  g_object_set_data(G_OBJECT(data), prop_action [type], NULL);
  g_object_set_data(G_OBJECT(data), prop_state  [type], NULL);
}

/* GKrellUIM */
static void
mode_data_flush(gpointer data)
{
  prop_data_flush(data,TYPE_MODE);
}

/* GKrellUIM */
static void
input_data_flush(gpointer data)
{
  prop_data_flush(data,TYPE_INPUT);
}

/* GKrellUIM */
static void
im_data_flush(gpointer data)
{
  prop_data_flush(data,TYPE_IM);
}

/*
 * taken from uim-svn3135/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
prop_button_append_menu(GtkWidget *button, const gint type,
                        const gchar *icon_name,
                        const gchar *label, const gchar *tooltip,
                        const gchar *action, const gchar *state)
{
  GList *icon_list, *label_list, *tooltip_list, *action_list, *state_list;

  /* GKrellUIM */
  icon_list    = g_object_get_data(G_OBJECT(button), prop_icon   [type]);
  label_list   = g_object_get_data(G_OBJECT(button), prop_label  [type]);
  tooltip_list = g_object_get_data(G_OBJECT(button), prop_tooltip[type]);
  action_list  = g_object_get_data(G_OBJECT(button), prop_action [type]);
  state_list   = g_object_get_data(G_OBJECT(button), prop_state  [type]);

  icon_list    = g_list_append(icon_list,    g_strdup(icon_name) );
  label_list   = g_list_append(label_list,   g_strdup(label)     );
  tooltip_list = g_list_append(tooltip_list, g_strdup(tooltip)   );
  action_list  = g_list_append(action_list,  g_strdup(action)    );
  state_list   = g_list_append(state_list,   g_strdup(state)     );

  /* GKrellUIM */
  g_object_set_data(G_OBJECT(button), prop_icon   [type], icon_list   );
  g_object_set_data(G_OBJECT(button), prop_label  [type], label_list  );
  g_object_set_data(G_OBJECT(button), prop_tooltip[type], tooltip_list);
  g_object_set_data(G_OBJECT(button), prop_action [type], action_list );
  g_object_set_data(G_OBJECT(button), prop_state  [type], state_list  );
}

/* GKrellUIM */
static void
im_button_append_menu(GtkWidget *button,
                      const gchar *icon_name,
                      const gchar *label, const gchar *tooltip,
                      const gchar *action, const gchar *state)
{
  prop_button_append_menu( button, TYPE_IM,
                           icon_name,
                           label, tooltip,
                           action, state );
}

/* GKrellUIM */
static void
mode_button_append_menu(GtkWidget *button,
                        const gchar *icon_name,
                        const gchar *label, const gchar *tooltip,
                        const gchar *action, const gchar *state)
{
  prop_button_append_menu( button, TYPE_MODE,
                           icon_name,
                           label, tooltip,
                           action, state );
}

/* GKrellUIM */
static void
input_button_append_menu(GtkWidget *button,
                        const gchar *icon_name,
                        const gchar *label, const gchar *tooltip,
                        const gchar *action, const gchar *state)
{
  prop_button_append_menu( button, TYPE_INPUT,
                           icon_name,
                           label, tooltip,
                           action, state );
}

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
 * taken from uim-svn3144/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_list_update(GtkWidget *widget, gchar **lines)
{
  /* XXX: remove button */
  int i;
  gchar **cols;
  gchar *charset;
  const gchar *indication_id, *iconic_label, *label, *tooltip_str;
  const gchar *action_id, *is_selected;
  /* XXX: remove prop_buttons & tool_buttons */
  /* GKrellUIM */
  gint branch_number;

  charset = get_charset(lines[1]);

  /* XXX: remove prop_buttons & tool_buttons */
  /* GKrellUIM */
  mode_data_flush(widget);
  input_data_flush(widget);
  im_data_flush(widget);

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );
  input_text = g_strdup( "-" );

  /* GKrellUIM */
  uim_init();
  if( uim_scm_symbol_value_bool("toolbar-show-action-based-switcher-button?") )
    branch_number = -1;
  else
    branch_number = 0;

  for (i = 0; lines[i] && strcmp("", lines[i]); i++) {
    gchar *utf8_str = convert_charset(charset, lines[i]);

    if (utf8_str != NULL) {
      cols = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      cols = g_strsplit(lines[i], "\t", 0);
    }

    if (cols && cols[0]) {
      if (!strcmp("branch", cols[0]) && has_n_strs(cols, 4)) {
        indication_id = cols[1];
        iconic_label  = cols[2];
        tooltip_str   = cols[3];
        /* XXX: remove button */
        /* GKrellUIM */
        branch_number++;
        switch( branch_number ) {
          case TYPE_MODE:
            mode_text  = g_strdup( iconic_label );
            break;
          case TYPE_INPUT:
            input_text = g_strdup( iconic_label );
            break;
          default:
            break;
        }
      } else if (!strcmp("leaf", cols[0]) && has_n_strs(cols, 7)) {
        indication_id = cols[1];
        iconic_label  = safe_gettext(cols[2]);
        label         = safe_gettext(cols[3]);
        tooltip_str   = safe_gettext(cols[4]);
        action_id     = cols[5];
        is_selected   = cols[6];
        /* XXX: remove button */
        /* GKrellUIM */
        switch( branch_number ) {
          case TYPE_IM:
            im_button_append_menu(widget,
                                  indication_id, label, tooltip_str,
                                  action_id, is_selected);
            break;
          case TYPE_MODE:
            mode_button_append_menu(widget,
                                    indication_id, label, tooltip_str,
                                    action_id, is_selected);
            break;
          case TYPE_INPUT:
            input_button_append_menu(widget,
                                     indication_id, label, tooltip_str,
                                     action_id, is_selected);
            break;
          default:
            break;
        }
      }
      g_strfreev(cols);
    }
  }

  /* XXX: remove tool_button */

  g_free(charset);
}

/*
 * taken from uim-svn3144/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
static void
helper_toolbar_prop_label_update(GtkWidget *widget, gchar **lines)
{
  /* XXX: remove button */
  unsigned int i;
  gchar **cols;
  gchar *charset, *indication_id, *iconic_label, *tooltip_str;
  /* XXX: remove prop_buttons */

  for (i = 0; lines[i] && strcmp("", lines[i]); i++)
    continue;

  /* XXX: remove prop_buttons */

  charset = get_charset(lines[1]);

  /* GKrellUIM */
  mode_text  = g_strdup( "?" );

  for (i = 2; lines[i] && strcmp("", lines[i]); i++) {
    if (charset) {
      gchar *utf8_str;
      utf8_str = g_convert(lines[i], strlen(lines[i]),
                           "UTF-8", charset,
                           NULL, /* gsize *bytes_read */
                           NULL, /*size *bytes_written */
                           NULL); /* GError **error*/
      cols = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      cols = g_strsplit(lines[i], "\t", 0);
    }

    if (has_n_strs(cols, 3)) {
      indication_id = cols[0];
      iconic_label  = cols[1];
      tooltip_str   = cols[2];
      /* XXX: remove prop_buttons */

      /* GKrellUIM */
      if( i - 2 > 0 ) {
        ;
      } else {
        mode_text = g_strdup( iconic_label );
      }
    }
    g_strfreev(cols);
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
 * taken from uim-svn3121/helper/toolbar-common-gtk.c
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
    else if (!strcmp("custom_reload_notify", lines[0])) {
      uim_prop_reload_configs();
      helper_toolbar_check_custom();
    }
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

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static gboolean
is_icon_registered(const gchar *name)
{
  GList *list;

  list = uim_icon_list;
  while (list) {
   if (!strcmp(list->data, name))
     return TRUE;
   list = list->next;
  }

  return FALSE;
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static gboolean
register_icon(const gchar *name)
{
  GtkIconSet *icon_set;
  GdkPixbuf *pixbuf;
  GString *filename;

  g_return_val_if_fail(uim_factory, FALSE);

  if (is_icon_registered(name))
    return TRUE;

  filename = g_string_new(UIM_PIXMAPSDIR "/");
  g_string_append(filename, name);
  g_string_append(filename, ".png");

  pixbuf = gdk_pixbuf_new_from_file(filename->str, NULL);
  if (!pixbuf) {
    g_string_free(filename, TRUE);
    return FALSE;
  }

  icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
  gtk_icon_factory_add(uim_factory, name, icon_set);

  g_list_append(uim_icon_list, g_strdup(name));

  g_string_free(filename, TRUE);
  gtk_icon_set_unref(icon_set);
  g_object_unref(G_OBJECT(pixbuf));

  return TRUE;
}

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
static void
init_icon(void)
{
  if (uim_factory)
    return;

  uim_factory = gtk_icon_factory_new();
  gtk_icon_factory_add_default(uim_factory);

  register_icon("switcher-icon");
  register_icon("uim-icon");
  register_icon("null");
}

/* GKrellUIM */
void helper_init(GtkWidget *widget )
{
  init_icon();

  helper_toolbar_check_custom();

  uim_fd = -1;

  uim_toolbar_check_helper_connection( widget );
  uim_helper_client_get_prop_list();
  uim_toolbar_get_im_list();

  g_atexit( uim_quit );
}
