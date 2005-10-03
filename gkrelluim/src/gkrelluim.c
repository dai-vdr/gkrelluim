/*
  GKrellUIM: GKrellM UIM helper Plugin
 
  Copyright (C) 2004-2005 dai <d+gkrelluim@vdr.jp>
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
void check_helper_connection( GtkWidget* );
extern int uim_fd;

/* GKrellUIM */
static GkrellmDecal *text_decal;
static GkrellmDecal *mode_text_decal;
static GkrellmDecal *input_text_decal;
gchar *mode_text  = "?";
gchar *input_text = "-";

static struct _Command {
  const gchar *desc;
  const gchar *config;
  GtkWidget   *entry;
  gchar       *exec;
  const gchar *pref_button_show_symbol;
  uim_bool     show;
} command[] = {
  { N_("im-switcher"),
    "im_switcher_exec",
    NULL,
    "uim-im-switcher-gtk",
    "toolbar-show-switcher-button?",
    UIM_TRUE },
  { N_("pref"),
    "pref_exec",
    NULL,
    "uim-pref-gtk",
    "toolbar-show-pref-button?",
    UIM_TRUE },
  { N_("dict"),
    "dict_exec",
    NULL,
    "uim-dict-gtk",
    "toolbar-show-dict-button?",
    UIM_FALSE },
  { N_("input-pad-ja"),
    "input_pad_ja_exec",
    NULL,
    "uim-input-pad-ja",
    "toolbar-show-input-pad-button?",
    UIM_FALSE },
  { N_("hand"),
    "hand_exec",
    NULL,
    "uim-tomoe-ja",
    "toolbar-show-handwriting-input-pad-button?",
    UIM_FALSE },
  { N_("help"),
    "help_exec",
    NULL,
    "uim-help",
    "toolbar-show-help-button?",
    UIM_FALSE }
};
static const gint COMMAND_LENGTH
  = sizeof( command ) / sizeof( struct _Command );

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

  if( !g_shell_parse_argv( command[ data ].exec, NULL, &argv, &err ) ) {
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
  GtkWidget *menu;
  GtkWidget *item[ COMMAND_LENGTH ];
  gint       i;
  gboolean   show = FALSE;

  menu = gtk_menu_new();

  uim_init();
  for( i = 0; i < COMMAND_LENGTH; i++ ) {
    if( uim_scm_symbol_value_bool( command[ i ].pref_button_show_symbol ) ) {
      show = TRUE;
      item[ i ] = gtk_menu_item_new_with_label( _(command[ i ].desc) );
      gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item[ i ] );
      g_signal_connect_swapped( G_OBJECT( item[ i ] ), "activate",
                                G_CALLBACK( exec_command ),
                                (gpointer)i );
      gtk_widget_show( item[ i ] );
    }
  }
  uim_quit();

  if( show ) {
    gtk_menu_popup( GTK_MENU( menu ), NULL, NULL,
                    NULL, NULL,
                    event->button, gtk_get_current_event_time() );
  }
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
  GkrellmDecalbutton *button;
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
	button,		/* Arg to callback function		*/
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
  }

  if( first_create ) {
    /* taken from toolbar-common-gtk.c (0.4.6beta2) */
    uim_fd = -1;
    check_helper_connection( vbox );
    uim_helper_client_get_prop_list();
  }
}

/*
 * taken from gkrellmms/options.c
 * modified for GKrellUIM
 */
static void
apply_gkrelluim_config( void ) {
  gint i;

  for( i = 0; i < COMMAND_LENGTH; i++ ) {
    g_free( command[ i ].exec );
    command[ i ].exec
      = g_strdup( gtk_entry_get_text( GTK_ENTRY( command[ i ].entry ) ) );
  }
}

/*
 * taken from gkrellmms/options.c
 * modified for GKrellUIM
 */
static void
save_gkrelluim_config( FILE *f ) {
  gint i;

  for( i = 0; i < COMMAND_LENGTH; i++ ) {
    fprintf( f, "%s %s %s\n", CONFIG_KEYWORD, command[ i ].config,
                command[ i ].exec );
  }
}

/*
 * taken from gkrellmms/options.c
 * modified for GKrellUIM
 */
static void
load_gkrelluim_config( gchar *arg ) {
  gchar config[ 64 ], item[ 256 ];
  gint  n, i;

  n = sscanf( arg, "%s %[^\n]", config, item );

  if( n == 2 ) {
    for( i = 0; i < COMMAND_LENGTH; i++ ) {
      if( strcmp( config, command[ i ].config ) == 0 ) {
        command[ i ].exec = g_strdup( item );
      }
    }
  }
}

/*
 * taken from alltraxclock.c, gkrellmms/options,c
 * modified for GKrellUIM
 */
static void
create_gkrelluim_tab( GtkWidget *tab_vbox ) {
  GtkWidget *tabs, *text;

  GtkWidget *frame;
  GtkWidget *vbox, *hbox, *zbox;

  GtkWidget *label;
  gchar     *label_text;
  gint       i;

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos( GTK_NOTEBOOK( tabs ), GTK_POS_TOP );
  gtk_box_pack_start( GTK_BOX( tab_vbox ), tabs, TRUE, TRUE, 0 );

  /* Config tab */
  frame = gtk_frame_new( NULL );
  gtk_container_border_width( GTK_CONTAINER( frame ), 3 );

  vbox = gtk_vbox_new( FALSE, 0 );
  gtk_container_border_width( GTK_CONTAINER( vbox ), 3 );

  hbox = gtk_hbox_new( FALSE, 5 );

  /* label vbox */
  zbox = gtk_vbox_new( FALSE, 0 );
  for( i = 0; i < COMMAND_LENGTH; i++ ) {
    label_text = g_strdup_printf( _("%s Executable:"), _(command[ i ].desc) );
    label = gtk_label_new( label_text );
    g_free( label_text );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE, FALSE, 0 );
  }
  gtk_box_pack_start( GTK_BOX( hbox ), zbox, FALSE, FALSE, 0 );

  /* entry vbox */
  zbox = gtk_vbox_new( FALSE, 0 );
  for( i = 0; i < COMMAND_LENGTH; i++ ) {
    command[ i ].entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( command[ i ].entry ), command[ i ].exec );
    gtk_entry_set_editable( GTK_ENTRY( command[ i ].entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), command[ i ].entry, TRUE, FALSE, 0 );
  }
  gtk_box_pack_start( GTK_BOX( hbox ), zbox, FALSE, FALSE, 0 );

  gtk_container_add( GTK_CONTAINER( vbox ), hbox );

  label = gtk_label_new( _("Config") );
  gtk_container_add( GTK_CONTAINER( frame ), vbox );
  gtk_notebook_append_page( GTK_NOTEBOOK( tabs ), frame, label );

  /* About tab */
  label_text = g_strdup_printf(
	_("GKrellUIM %s\n"
	"GKrellM UIM helper Plugin\n\n"
	"Copyright (C) 2004-2005 dai\n"
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

  create_gkrelluim,		/* The create_plugin() function */
  update_gkrelluim,		/* The update_plugin() function */

  create_gkrelluim_tab,		/* The create_plugin_tab() config function */
  apply_gkrelluim_config,	/* The apply_plugin_config() function */

  save_gkrelluim_config,	/* The save_plugin_config() function */
  load_gkrelluim_config,	/* The load_plugin_config() function */
  CONFIG_KEYWORD,		/* config keyword */

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
