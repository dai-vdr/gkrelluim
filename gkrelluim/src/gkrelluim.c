/*
 * GKrellUIM 0.0.4 (20050627)
 *
 * dai <d+gkrelluim@vdr.jp>
 */

/* GKrellM */
#include <gkrellm2/gkrellm.h>
static GkrellmMonitor *monitor;
static GkrellmPanel *panel;
static GkrellmDecal *decal;
static gint style_id;

/* UIM */
extern int uim_fd;

/* GKrellUIM */
#define CONFIG_KEYWORD "gkrelluim"
static GkrellmDecal *text_decal;
static GkrellmDecal *mode_text_decal;
static GkrellmDecal *input_text_decal;
gchar *mode_text  = "?";
gchar *input_text = "-";

static GtkWidget *im_switcher_entry;
static GtkWidget *pref_entry;
static GtkWidget *dict_entry;
static GtkWidget *input_pad_ja_entry;
static GtkWidget *hand_entry;
static GtkWidget *help_entry;
static gchar *im_switcher_exec;
static gchar *pref_exec;
static gchar *dict_exec;
static gchar *input_pad_ja_exec;
static gchar *hand_exec;
static gchar *help_exec;
enum {
  IM_SWITCHER_ID,
  PREF_ID,
  DICT_ID,
  INPUT_PAD_JA_ID,
  HAND_ID,
  HELP_ID
};

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
 * taken from gkrellm2-demos/demo3.c and gkrellmms/options.c,
 * modified for GKrellUIM
 */
static void
exec_command( gint data ) {
  gchar  **argv = 0;
  GError  *err  = NULL;
  gboolean res;
  gchar   *exec = NULL;

  switch( data ) {
    case IM_SWITCHER_ID:
      exec = im_switcher_exec;
      break;
    case PREF_ID:
      exec = pref_exec;
      break;
    case DICT_ID:
      exec = dict_exec;
      break;
    case INPUT_PAD_JA_ID:
      exec = input_pad_ja_exec;
      break;
    case HAND_ID:
      exec = hand_exec;
      break;
    case HELP_ID:
      exec = help_exec;
      break;
    default:
      /* unreachable */
      break;
  }

  if( !g_shell_parse_argv( exec, NULL, &argv, &err ) ) {
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
  GtkWidget *im_switcher_item;
  GtkWidget *pref_item;
  GtkWidget *dict_item;
  GtkWidget *input_pad_ja_item;
  GtkWidget *hand_item;
  GtkWidget *help_item;

  menu = gtk_menu_new();

  im_switcher_item  = gtk_menu_item_new_with_label( "im-switcher" );
  pref_item         = gtk_menu_item_new_with_label( "pref" );
  dict_item         = gtk_menu_item_new_with_label( "dict" );
  input_pad_ja_item = gtk_menu_item_new_with_label( "input-pad-ja" );
  hand_item         = gtk_menu_item_new_with_label( "hand" );
  help_item         = gtk_menu_item_new_with_label( "help" );

  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), im_switcher_item );
  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), pref_item );
  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), dict_item );
  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), input_pad_ja_item );
  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), hand_item );
  gtk_menu_shell_append( GTK_MENU_SHELL( menu ), help_item );

  g_signal_connect_swapped( G_OBJECT( im_switcher_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)IM_SWITCHER_ID );
  g_signal_connect_swapped( G_OBJECT( pref_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)PREF_ID );
  g_signal_connect_swapped( G_OBJECT( dict_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)DICT_ID );
  g_signal_connect_swapped( G_OBJECT( input_pad_ja_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)INPUT_PAD_JA_ID );
  g_signal_connect_swapped( G_OBJECT( hand_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)HAND_ID );
  g_signal_connect_swapped( G_OBJECT( help_item ), "activate",
                            G_CALLBACK( exec_command ),
                            (gpointer)HELP_ID );

  gtk_widget_show( im_switcher_item );
  gtk_widget_show( pref_item );
  gtk_widget_show( dict_item );
  gtk_widget_show( input_pad_ja_item );
  gtk_widget_show( hand_item );
  gtk_widget_show( help_item );

  gtk_menu_popup( GTK_MENU(menu), NULL, NULL,
                  NULL, NULL,
                  event->button, gtk_get_current_event_time() );
}

/*
 * taken from gkrellm2-demos/demo[234].c, modified for GKrellUIM
 */
static void
create_gkrelluim( GtkWidget *vbox, gint first_create ) {
  GkrellmStyle     *style;
  GkrellmTextstyle *text_style;

  /* GKrellUIM */
  GkrellmTextstyle   *text_style_alt;
  GkrellmDecalbutton *button;
  gint               x;

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
	0);

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
  if ( first_create ) {
    g_signal_connect(G_OBJECT (panel->drawing_area), "expose_event",
                     G_CALLBACK(panel_expose_event), NULL);
  }

  if ( first_create ) {
    /* taken from toolbar-common-gtk.c (0.4.6beta2) */
    uim_fd = -1;
    check_helper_connection(vbox);
    uim_helper_client_get_prop_list();
  }
}

/*
 * taken from gkrellmms/options.c, modified for GKrellUIM
 */
static void
apply_gkrelluim_config( void ) {
  g_free( im_switcher_exec );
  g_free( pref_exec );
  g_free( dict_exec );
  g_free( input_pad_ja_exec );
  g_free( hand_exec );
  g_free( help_exec );
  im_switcher_exec  = g_strdup( gtk_entry_get_text( GTK_ENTRY( im_switcher_entry ) ) );
  pref_exec         = g_strdup( gtk_entry_get_text( GTK_ENTRY( pref_entry ) ) );
  dict_exec         = g_strdup( gtk_entry_get_text( GTK_ENTRY( dict_entry ) ) );
  input_pad_ja_exec = g_strdup( gtk_entry_get_text( GTK_ENTRY( input_pad_ja_entry ) ) );
  hand_exec         = g_strdup( gtk_entry_get_text( GTK_ENTRY( hand_entry ) ) );
  help_exec         = g_strdup( gtk_entry_get_text( GTK_ENTRY( help_entry ) ) );
}

/*
 * taken from gkrellmms/options.c, modified for GKrellUIM
 */
static void
save_gkrelluim_config( FILE *f ) {
  fprintf( f, "%s im_switcher_exec %s\n",  CONFIG_KEYWORD, im_switcher_exec );
  fprintf( f, "%s pref_exec %s\n",         CONFIG_KEYWORD, pref_exec );
  fprintf( f, "%s dict_exec %s\n",         CONFIG_KEYWORD, dict_exec );
  fprintf( f, "%s input_pad_ja_exec %s\n", CONFIG_KEYWORD, input_pad_ja_exec );
  fprintf( f, "%s hand_exec %s\n",         CONFIG_KEYWORD, hand_exec );
  fprintf( f, "%s help_exec %s\n",         CONFIG_KEYWORD, help_exec );
}

/*
 * taken from gkrellmms/options.c, modified for GKrellUIM
 */
static void
load_gkrelluim_config( gchar *arg ) {
  gchar config[ 64 ], item[ 256 ];
  gint  n;

  n = sscanf( arg, "%s %[^\n]", config, item );

  if( n == 2 ) {
    if( strcmp( config, "im_switcher_exec" ) == 0 ) {
      im_switcher_exec = g_strdup( item );
    }
    if( strcmp( config, "pref_exec" ) == 0 ) {
      pref_exec = g_strdup( item );
    }
    if( strcmp( config, "dict_exec" ) == 0 ) {
      dict_exec = g_strdup( item );
    }
    if( strcmp( config, "input_pad_ja_exec" ) == 0 ) {
      input_pad_ja_exec = g_strdup( item );
    }
    if( strcmp( config, "hand_exec" ) == 0 ) {
      hand_exec = g_strdup( item );
    }
    if( strcmp( config, "help_exec" ) == 0 ) {
      help_exec = g_strdup( item );
    }
  }
}

/*
 * taken from alltraxclock.c and gkrellmms/options,
 * modified for GKrellUIM
 */
static void
create_gkrelluim_tab( GtkWidget *tab_vbox ) {
  GtkWidget *tabs, *text;

  GtkWidget *frame;
  GtkWidget *vbox, *hbox, *zbox;

  GtkWidget *label;
  gchar     *plugin_about_text;

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos( GTK_NOTEBOOK( tabs ), GTK_POS_TOP );
  gtk_box_pack_start( GTK_BOX( tab_vbox ), tabs, TRUE, TRUE, 0 );

  /* Config tab */
  frame = gtk_frame_new( NULL );
  gtk_container_border_width( GTK_CONTAINER( frame ), 3 );

  vbox = gtk_vbox_new( FALSE, 0 );
  gtk_container_border_width( GTK_CONTAINER( vbox ), 3 );

  hbox = gtk_hbox_new( FALSE, 5 );

  zbox = gtk_vbox_new( FALSE, 0 );
    label = gtk_label_new( "uim-im-switcher Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );

    label = gtk_label_new( "uim-pref Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );

    label = gtk_label_new( "uim-dict Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );

    label = gtk_label_new( "uim-input-pad-ja Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );

    label = gtk_label_new( "uim-hand Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );

    label = gtk_label_new( "uim-help Executable:" );
    gtk_box_pack_start( GTK_BOX( zbox ), label, TRUE,  FALSE, 0 );
  gtk_box_pack_start( GTK_BOX( hbox ), zbox, FALSE,  FALSE, 0 );

  zbox = gtk_vbox_new( FALSE, 0 );
    im_switcher_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( im_switcher_entry ), im_switcher_exec );
    gtk_entry_set_editable( GTK_ENTRY( im_switcher_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), im_switcher_entry, TRUE,  FALSE, 0 );

    pref_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( pref_entry ), pref_exec );
    gtk_entry_set_editable( GTK_ENTRY( pref_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), pref_entry, TRUE,  FALSE, 0 );

    dict_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( dict_entry ), dict_exec );
    gtk_entry_set_editable( GTK_ENTRY( dict_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), dict_entry, TRUE,  FALSE, 0 );

    input_pad_ja_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( input_pad_ja_entry ), input_pad_ja_exec );
    gtk_entry_set_editable( GTK_ENTRY( input_pad_ja_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), input_pad_ja_entry, TRUE,  FALSE, 0 );

    hand_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( hand_entry ), hand_exec );
    gtk_entry_set_editable( GTK_ENTRY( hand_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), hand_entry, TRUE,  FALSE, 0 );

    help_entry = gtk_entry_new_with_max_length( 255 );
    gtk_entry_set_text( GTK_ENTRY( help_entry ), help_exec );
    gtk_entry_set_editable( GTK_ENTRY( help_entry ), TRUE );
    gtk_box_pack_start( GTK_BOX( zbox ), help_entry, TRUE,  FALSE, 0 );
  gtk_box_pack_start( GTK_BOX( hbox ), zbox, FALSE,  FALSE, 0 );

  gtk_container_add( GTK_CONTAINER( vbox ), hbox );

  label = gtk_label_new( "Config" );
  gtk_container_add( GTK_CONTAINER( frame ), vbox );
  gtk_notebook_append_page( GTK_NOTEBOOK( tabs ), frame, label );

  /* About tab */
  plugin_about_text = g_strdup(
	"GKrellUIM "PACKAGE_VERSION"\n"
	"GKrellM UIM helper Plugin\n\n"
	"Copyright (C) 2004,2005 dai\n"
	"d+gkrelluim@vdr.jp\n"
	"http://vdr.jp/d/gkrelluim.html\n\n"
	"Released under the GNU General Public License\n" );
  text = gtk_label_new( plugin_about_text );
  g_free( plugin_about_text );
  label = gtk_label_new( "About" );
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
  im_switcher_exec  = g_strdup( "uim-im-switcher-gtk" );
  pref_exec         = g_strdup( "uim-pref-gtk" );
  dict_exec         = g_strdup( "uim-dict-gtk" );
  input_pad_ja_exec = g_strdup( "uim-input-pad-ja" );
  hand_exec         = g_strdup( "uim-tomoe-gtk" );
  help_exec         = g_strdup( "uim-help" );

  style_id = gkrellm_add_meter_style( &plugin_gkrelluim, "gkrelluim" );
  monitor = &plugin_gkrelluim;

  return &plugin_gkrelluim;
}
