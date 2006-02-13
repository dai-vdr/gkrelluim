/*
  GKrellUIM: GKrellM UIM helper Plugin

  Copyright (C) 2004-2006 dai <d+gkrelluim@vdr.jp>
  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* GKrellM */
#define PACKAGE        "gkrelluim"
#define CONFIG_KEYWORD PACKAGE
#define STYLE_NAME     PACKAGE

#include <gkrellm2/gkrellm.h>
static GkrellmMonitor *monitor;
static GkrellmPanel   *panel;
static GkrellmDecal   *decal;
static gint style_id;

/* UIM */
#include <uim/uim.h>            /* uim_bool */
#include <uim/uim-helper.h>     /* uim_helper_client_get_prop_list */
#include <uim/uim-compat-scm.h> /* uim_scm_symbol_value_bool */
extern int uim_fd;
#define OBJECT_DATA_IM_BUTTON "IM_BUTTON"

/* GKrellUIM */
static GkrellmDecal *text_decal;
static GkrellmDecal *mode_text_decal;
static GkrellmDecal *input_text_decal;
gchar *mode_text  = "?";
gchar *input_text = "-";

void create_im_menu(GtkWidget*, GtkWidget*);
void im_menu_button_new(GtkWidget*, GtkWidget*);

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 */
/* FIXME! command menu and buttons should be customizable. */
static struct _CommandEntry {
  const gchar *desc;
  const gchar *label;
  const gchar *icon;
  const gchar *command;
  const gchar *custom_button_show_symbol;
  uim_bool show_button;
} command_entry[] = {
  {N_("Switch input method"),
   NULL,
   "switcher-icon",
   "uim-im-switcher-gtk &",
   "toolbar-show-switcher-button?",
   UIM_FALSE},

  {N_("Preference"),
   NULL,
   GTK_STOCK_PREFERENCES,
   "uim-pref-gtk &",
   "toolbar-show-pref-button?",
   UIM_FALSE},

  {N_("Japanese dictionary editor"),
   "Dic",
   NULL,
   "uim-dict-gtk &",
   "toolbar-show-dict-button?",
   UIM_FALSE},

  {N_("Input pad"),
   "Pad",
   NULL,
   "uim-input-pad-ja &",
   "toolbar-show-input-pad-button?",
   UIM_FALSE},

  {N_("Handwriting input pad"),
   "Hand",
   NULL,
   "uim-tomoe-gtk &",
   "toolbar-show-handwriting-input-pad-button?",
   UIM_FALSE},

  {N_("Help"),
   NULL,
   GTK_STOCK_HELP,
   "uim-help &",
   "toolbar-show-help-button?",
   UIM_FALSE}
};
static gint command_entry_len = sizeof(command_entry) / sizeof(struct _CommandEntry);

/*
 * taken from uim-svn3105/helper/toolbar-common-gtk.c
 * modified for GKrellUIM
 */
/* GkrellUIM: static */ void
helper_toolbar_check_custom()
{
  int i;

  for (i = 0; i < command_entry_len; i++)
    command_entry[i].show_button =
      uim_scm_symbol_value_bool(command_entry[i].custom_button_show_symbol);
}

/*
 * taken from gkrellm2-demos/demo2.c
 */
static gint
panel_expose_event( GtkWidget *widget, GdkEventExpose *ev ) {
  gdk_draw_pixmap( widget->window,
    widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ],
    panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
    ev->area.width, ev->area.height );
  return FALSE;
}

/*
 * taken from gkrellm2-demos/demo2.c, gkrellmms/gkrellmms.c
 * modified for GKrellUIM
 */
static void
update_gkrelluim( void ) {
  /* GKrellUIM */
  gchar *mode_label  = NULL;
  gchar *input_label = NULL;

  mode_label  = g_strdup_printf( "%s", mode_text  );
  input_label = g_strdup_printf( "%s", input_text );

  gkrellm_draw_decal_text( panel, mode_text_decal,  mode_label,  0 );
  gkrellm_draw_decal_text( panel, input_text_decal, input_label, 0 );

  gkrellm_draw_panel_layers( panel );

  g_free( mode_label  );
  g_free( input_label );
}

/*
 * taken from gkrellm2-demos/demo3.c, gkrellmms/options.c
 * modified for GKrellUIM
 */
static void
exec_command( gint data ) {
  gchar  **argv = 0;
  GError  *err  = NULL;
  gboolean res;

  if( !g_shell_parse_argv( command_entry[ data ].command, NULL, &argv, &err ) ) {
    gkrellm_message_window( "GKrellUIM Error", err->message, NULL );
    g_error_free( err );
  } else {
    res = g_spawn_async( NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                         NULL, NULL, NULL, &err );
    if( !res && err ) {
      gkrellm_message_window( "GKrellUIM Error", err->message, NULL );
      g_error_free( err );
    }
  }
}

/*
 * GKrellUIM (gtk+-2.0.x-tut/sec-manualmenuexample.html)
 */
static void
cb_menu_button( GkrellmDecalbutton *button, GdkEventButton *event ) {
  GtkWidget *menu      = gtk_menu_new();
  GtkWidget *separator = gtk_separator_menu_item_new();
  GtkWidget *item[ command_entry_len ];
  gint       i;

  create_im_menu( menu, GTK_WIDGET(event) );

  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), separator );
  gtk_widget_show( separator );

  for( i = 0; i < command_entry_len; i++ ) {
    if( uim_scm_symbol_value_bool( command_entry[ i ].custom_button_show_symbol ) ) {
      item[ i ] = gtk_menu_item_new_with_label( _(command_entry[ i ].desc) );
      gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item[ i ] );
      g_signal_connect_swapped( G_OBJECT( item[ i ] ), "activate",
                                G_CALLBACK( exec_command ),
                                (gpointer)i );
      gtk_widget_show( item[ i ] );
    }
  }

  gtk_menu_popup( GTK_MENU( menu ), NULL, NULL,
                  NULL, NULL,
                  event->button, gtk_get_current_event_time() );
}

/*
 * taken from gkrellm2-demos/demo[234].c
 * modified for GKrellUIM
 */
static void
create_gkrelluim( GtkWidget *vbox, gint first_create ) {
  GkrellmStyle     *style;
  GkrellmTextstyle *text_style;

  /* GKrellUIM */
  GkrellmTextstyle   *text_style_alt;
  GkrellmDecalbutton *button /* = NULL */;
  gint                x;

  if( first_create ) {
    panel = gkrellm_panel_new0();
  }

  style = gkrellm_meter_style( style_id );

  /* GKrellUIM */
  text_style = gkrellm_meter_textstyle( style_id );
  text_style_alt = gkrellm_meter_alt_textstyle( style_id );
  decal = gkrellm_create_decal_text( panel, "Aq", text_style, style,
                                     -1, -1, -1);

  /* gkrellm2-demos/demo2.c */
  text_decal = gkrellm_create_decal_text( panel, "Aq", text_style_alt, style,
	7,
	5,
	0 );

  /* GKrellUIM */
  mode_text_decal = gkrellm_create_decal_text( panel, "Aq", text_style, style,
	0 + 5,
	5,
	0 );
  gkrellm_draw_decal_text( panel, mode_text_decal, mode_text, 0 );

  /* GKrellUIM */
  input_text_decal = gkrellm_create_decal_text( panel, "Aq", text_style, style,
	20 + 5,
	5,
	0 );
  gkrellm_draw_decal_text( panel, input_text_decal, input_text, 0 );

  x = decal->x + decal->w;
  button = gkrellm_make_scaled_button( panel,
	NULL,		/* GkrellmPiximage image		*/
	cb_menu_button,	/* Button clicked callback function	*/
	vbox,		/* Arg to callback function		*/
	FALSE,		/* auto_hide				*/
	FALSE,		/* set_default_border			*/
	0,		/* Image depth				*/
	0,		/* Initial out frame			*/
	0,		/* Pressed frame			*/
	x - 15 - 2,	/* x position of button			*/
	2 + 2,		/* y position of button			*/
	15,		/* Width for scaling the button		*/
	15 );		/* Height for scaling the button	*/

  gkrellm_panel_configure( panel, NULL, style );

  gkrellm_panel_create( vbox, monitor, panel );
  if( first_create ) {
    g_signal_connect( G_OBJECT( panel->drawing_area ), "expose_event",
                      G_CALLBACK( panel_expose_event ), NULL );

    uim_init();
    gkrellm_disable_plugin_connect( monitor, uim_quit );

    g_object_set_data(G_OBJECT(vbox), OBJECT_DATA_IM_BUTTON, button);

    helper_toolbar_check_custom();

    uim_fd = -1;

    uim_toolbar_check_helper_connection(vbox);
    uim_helper_client_get_prop_list();
    uim_toolbar_get_im_list();
  }
}

/*
 * taken from alltraxclock.c, gkrellmms/options,c
 * modified for GKrellUIM
 */
static void
create_gkrelluim_tab( GtkWidget *tab_vbox ) {
  GtkWidget *tabs, *text;

  GtkWidget *label;
  gchar     *label_text;

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos( GTK_NOTEBOOK( tabs ), GTK_POS_TOP );
  gtk_box_pack_start( GTK_BOX( tab_vbox ), tabs, TRUE, TRUE, 0 );

  /* About tab */
  label_text = g_strdup_printf(
	_("GKrellUIM %s\n"
	"GKrellM UIM helper Plugin\n\n"
	"Copyright (C) 2004-2006 dai\n"
	"d+gkrelluim@vdr.jp\n"
	"http://vdr.jp/d/gkrelluim.html\n\n"
	"Released under the GNU General Public License\n"), PACKAGE_VERSION );
  text = gtk_label_new( label_text );
  g_free( label_text );
  label = gtk_label_new( _("About") );
  gtk_notebook_append_page( GTK_NOTEBOOK( tabs ), text, label );
}

/*
 * plugin_gkrelluim
 */
static GkrellmMonitor plugin_gkrelluim = {
  "GKrellUIM",	/* Name, for config tab.	*/
  0,		/* ID, 0 if a plugin		*/

  create_gkrelluim,	/* The create_plugin() function */
  update_gkrelluim,	/* The update_plugin() function */

  create_gkrelluim_tab,	/* The create_plugin_tab() config function */
  NULL, 		/* The apply_plugin_config() function */

  NULL, 		/* The save_plugin_config() function */
  NULL, 		/* The load_plugin_config() function */
  CONFIG_KEYWORD,	/* config keyword */

  NULL,		/* Undefined 2	*/
  NULL,		/* Undefined 1	*/
  NULL,		/* Undefined 0	*/

  MON_UPTIME,	/* Insert plugin before this monitor.	*/
  NULL,		/* Handle if a plugin, filled in by GKrellM	*/
  NULL		/* path if a plugin, filled in by GKrellM	*/
};

/*
 * gkrellm_init_plugin
 */
GkrellmMonitor *gkrellm_init_plugin( void ) {
#ifdef ENABLE_NLS
  bind_textdomain_codeset( PACKAGE, "UTF-8" );
#endif

  style_id = gkrellm_add_meter_style( &plugin_gkrelluim, STYLE_NAME );
  monitor = &plugin_gkrelluim;

  return &plugin_gkrelluim;
}
