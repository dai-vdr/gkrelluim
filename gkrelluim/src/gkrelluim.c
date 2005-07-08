/*
 * GKrellUIM 0.0.3 (20050227)
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
#include <uim/uim.h>
#include <uim/uim-helper.h>
#define OBJECT_DATA_SIZE_GROUP "SIZE_GROUP"

static unsigned int read_tag;
static int uim_fd;

/* GKrellUIM */
#define CONFIG_KEYWORD "gkrelluim"
static gchar *status_text = "?";
static GkrellmDecal *text_decal;
static GtkWidget *im_switcher_entry;
static GtkWidget *pref_entry;
static GtkWidget *dict_entry;
static GtkWidget *input_pad_ja_entry;
static GtkWidget *hand_entry;
static GtkWidget *help_entry;
gchar *im_switcher_exec;
gchar *pref_exec;
gchar *dict_exec;
gchar *input_pad_ja_exec;
gchar *hand_exec;
gchar *help_exec;
enum {
  IM_SWITCHER_ID,
  PREF_ID,
  DICT_ID,
  INPUT_PAD_JA_ID,
  HAND_ID,
  HELP_ID
};

/*
 * taken from toolbar-common-gtk.c (0.4.6beta2)
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
 * taken from toolbar-common-gtk.c (0.4.6beta2)
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
 * taken from toolbar-common-gtk.c (0.4.6beta2), modified for GKrellUIM
 */
static void
helper_toolbar_prop_list_update(GtkWidget *widget, gchar **tmp)
{
  int i = 0;
  gchar **tmp2 = NULL;
  gchar *charset = NULL;

  charset = get_charset(tmp[1]);

  /* XXX: remove menu_buttons */

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
        if( i == 2 ) {
          status_text = g_strdup( tmp2[1] );
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
 * taken from toolbar-common-gtk.c (0.4.6beta2)
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
        status_text = g_strdup( pair[0] );
      }
    }
    g_strfreev(pair);
    i++;
  }

  if(charset)
    g_free(charset);
}

/*
 * taken from toolbar-common-gtk.c (0.4.6beta2)
 */
static void
helper_disconnect_cb(void)
{
  uim_fd = -1;
  g_source_remove(read_tag);
}

/*
 * taken from toolbar-common-gtk.c (0.4.6beta2),
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
 * taken from toolbar-common-gtk.c (0.4.6beta2), modified for GKrellUIM
 */
static gboolean
fd_read_cb(GIOChannel *channel, GIOCondition c, gpointer p)
{
  gchar *tmp;
  int fd = g_io_channel_unix_get_fd(channel);
  GtkWidget *widget = GTK_WIDGET(p);

  uim_helper_read_proc(fd);

  while ((tmp = uim_helper_get_message())) {
    helper_toolbar_parse_helper_str(widget, tmp);
    free(tmp); tmp = NULL;
  }

  return TRUE;
}

/*
 * taken from toolbar-common-gtk.c (0.4.6beta2)
 */
static void
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

/*
 * taken from gkrellm2-demos/demo2.c
 */
static gint panel_expose_event( GtkWidget *widget, GdkEventExpose *ev ) {
  gdk_draw_pixmap( widget->window,
    widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ],
    panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
    ev->area.width, ev->area.height );
  return FALSE;
}

/*
 * taken from gkrellm2-demos/demo2.c, modified for GKrellUIM
 */
static void update_gkrelluim() {
  gchar *label = NULL;

  label = g_strdup_printf( "UIM %s", status_text );

  decal->x_off = 0;
  gkrellm_draw_decal_text( panel, decal, label, -1 );

  gkrellm_draw_panel_layers( panel );

  g_free( label );
}

/*
 * taken from gkrellm2-demos/demo3.c and gkrellmms/options.c,
 * modified for GKrellUIM
 */
static void exec_command( GtkWidget *w, gint data ) {
  gchar  **argv = 0;
  GError  *err  = NULL;
  gboolean res;
  gchar   *exec;

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
 * taken from ggtk+-2.0.x-tut/sec-itemfactoryexample.html,
 * modified for GKrellUIM
 */
static GtkItemFactoryEntry menu_items[] = {
  { "/im-switcher",  NULL, exec_command, IM_SWITCHER_ID,  NULL, NULL },
  { "/pref",         NULL, exec_command, PREF_ID,         NULL, NULL },
  { "/dict",         NULL, exec_command, DICT_ID,         NULL, NULL },
  { "/input-pad-ja", NULL, exec_command, INPUT_PAD_JA_ID, NULL, NULL },
  { "/hand",         NULL, exec_command, HAND_ID,         NULL, NULL },
  { "/help",         NULL, exec_command, HELP_ID,         NULL, NULL },
};
static gint nmenu_items = sizeof( menu_items ) / sizeof( menu_items[0] );

/*
 * taken from gtk+-2.0.x-tut/sec-itemfactoryexample.html,
 * modified for GKrellUIM
 */
static GtkWidget *get_menubar_menu( GtkWidget *window ) {
  GtkItemFactory *item_factory;
  GtkAccelGroup  *accel_group;

  accel_group  = gtk_accel_group_new();
  item_factory = gtk_item_factory_new( GTK_TYPE_MENU_BAR, "<main>", accel_group 
);
  gtk_item_factory_create_items( item_factory, nmenu_items, menu_items, NULL );
  gtk_window_add_accel_group( GTK_WINDOW( window ), accel_group );

  return gtk_item_factory_get_widget( item_factory, "<main>" );
}

/*
 * taken from gkrellm2-demos/demo3.c and gkrellmms/options.c,
 * modified for GKrellUIM
 */
static void
cb_button( GkrellmDecalbutton *button, GdkEventButton *event ) {
  GtkItemFactory *item_factory;
  GtkWidget      *menu;

  item_factory = gtk_item_factory_new( GTK_TYPE_MENU, "<main>", NULL );
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
  menu = gtk_item_factory_get_widget (item_factory, "<main>");

  gtk_menu_popup( GTK_MENU(menu), NULL, NULL,
                  NULL, NULL,
                  event->button, gtk_get_current_event_time() );
}

/*
 * taken from gkrellm2-demos/demo[234].c, modified for GKrellUIM
 */
static void create_gkrelluim( GtkWidget *vbox, gint first_create ) {
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

  x = decal->x + decal->w;
  button = gkrellm_make_scaled_button( panel,
	NULL,		/* GkrellmPiximage image		*/
	cb_button,	/* Button clicked callback function	*/
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
void apply_gkrelluim_config() {
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
void save_gkrelluim_config( FILE *f ) {
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
void load_gkrelluim_config( gchar *arg ) {
  gchar config[ 64 ], item[ 256 ]; /*, command[ 64 ]; */
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
static void create_gkrelluim_tab( GtkWidget *tab_vbox ) {
  GtkWidget *tabs, *text;

  GtkWidget *frame;
  GtkWidget *vbox, *hbox, *zbox;
  GtkWidget *im_switcher_hbox,  *pref_hbox, *dict_hbox,
            *input_pad_ja_hbox, *hand_hbox, *help_hbox;

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
