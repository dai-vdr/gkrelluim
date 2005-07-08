/*
 * GKrellUIM 0.0.1 (20040923)
 *
 * dai@kip <dai@kip.iis.toyama-u.ac.jp>
 */

// GKrellM
#include <gkrellm2/gkrellm.h>

static GkrellmMonitor *monitor;
static GkrellmPanel *panel;
static GkrellmDecal *decal;
static gint style_id;

// UIM
#include <uim/uim.h>
#include <uim/uim-helper.h>

static int read_tag;
static int uim_fd;

// GKrellUIM
static gchar *status_text = "?";

/*
 * taken from helper-toolbar-common-gtk.c, modified for GKrellUIM
 */
static void
helper_applet_prop_list_update(gchar **tmp)
{
  int i = 0;
  gchar **tmp2 = NULL;
  gchar *charset = NULL;

  tmp2 = g_strsplit(tmp[1], "=", 0);

  if(tmp2 && tmp2[0] && tmp2[1] && strcmp("charset", tmp2[0]) == 0) {
    charset = g_strdup(tmp2[1]);
    g_strfreev(tmp2);
  } else {
    g_strfreev(tmp2);
    return;
  }

  /* XXX: remove menu_buttons */

  while(tmp[i] && strcmp("", tmp[i]) != 0) {
    if (charset) {
      char* utf8_str;
      utf8_str = g_convert(tmp[i], strlen(tmp[i]),
                           "UTF-8", charset,
                           NULL, /* gsize *bytes_read */
                           NULL, /*size *bytes_written */
                           NULL); /* GError **error*/

      tmp2 = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      tmp2 = g_strsplit(tmp[i], "\t", 0);
    }

    if(tmp2 && tmp2[0])
    {
      if(strcmp("branch", tmp2[0]) == 0) {
        /* XXX: remove menu_buttons */
        // GKrellUIM
        if( i == 2 ) {
          gchar **tmp3 = NULL;
          tmp3 = g_strsplit( tmp[i], "\t", 0 );
          status_text = g_strdup( tmp3[1] );
          g_strfreev( tmp3 );
        }
      }
      /* XXX: remove leaf */
      g_strfreev(tmp2);
    }
    i++;
  }
  if(charset)
    g_free(charset);
}

/*
 * taken from helper-toolbar-common-gtk.c, modified for GKrellUIM
 */
static void
helper_applet_prop_label_update(gchar **tmp)
{
  unsigned int i = 0;
  gchar **tmp2 = NULL;
  gchar *charset = NULL;

  if(tmp && tmp[1] ) {
    tmp2 = g_strsplit(tmp[1], "=", 0);
  } else {
    return;
  }

  if(tmp2 && tmp2[0] && tmp2[1] && strcmp("charset", tmp2[0]) == 0) {
    charset = g_strdup(tmp2[1]);
    g_strfreev(tmp2);
  } else {
    g_strfreev(tmp2);
    return;
  }

  while(tmp[i] && strcmp("", tmp[i]) != 0) {
    i++;
  }

  /* XXX: remove menu_buttons */

  i = 1; /* resetting temporary variable */

  while(tmp[i] && strcmp("", tmp[i]) != 0) {

    if (charset) {
      char* utf8_str;
      utf8_str = g_convert(tmp[i], strlen(tmp[i]),
                           "UTF-8", charset,
                           NULL, /* gsize *bytes_read */
                           NULL, /*size *bytes_written */
                           NULL); /* GError **error*/

      tmp2 = g_strsplit(utf8_str, "\t", 0);
      g_free(utf8_str);
    } else {
      tmp2 = g_strsplit(tmp[i], "\t", 0);
    }
   
    if(tmp2 && tmp2[0] && tmp2[1])
    {
      /* XXX: remove menu_buttons */
      // GKrellUIM
      if( i - 2 > 0 ) {
        ;
      } else {
        if( strcmp( "UTF-8", charset ) ) {
          status_text = g_strdup( tmp2[0] );
        } else {
          status_text = g_convert( tmp2[0], strlen( tmp2[0] ),
                                   "UTF-8", charset, NULL, NULL, NULL );
        }
      }
    }
    g_strfreev(tmp2);
    i++;
  }

  if(charset)
    g_free(charset);
}

/*
 * taken from helper-toolbar-common-gtk.c
 */
static void
helper_disconnect_cb(void)
{
  uim_fd = -1;
  gdk_input_remove(read_tag);
}

/*
 * taken from helper-toolbar-common-gtk.c
 */
static void
helper_applet_parse_helper_str(char *str)
{
  gchar **lines;
    
  lines = g_strsplit(str, "\n", 0);

  if(lines && lines[0]) {
    if( strcmp("prop_list_update", lines[0]) == 0) {
      helper_applet_prop_list_update(lines);
    } else if( strcmp("prop_label_update", lines[0]) == 0) {
      helper_applet_prop_label_update(lines);
    }
    g_strfreev(lines);
  }
}

/*
 * taken from helper-toolbar-common-gtk.c
 */
static void
uim_applet_fd_read_cb(gpointer p, gint fd, GdkInputCondition c)
{
  char *tmp;
  
  uim_helper_read_proc(fd);
  while ((tmp = uim_helper_get_message())) {
    helper_applet_parse_helper_str(tmp);
    g_free(tmp); tmp = NULL;
  }
}

/*
 * taken from helper-toolbar-common-gtk.c
 */
static void
check_helper_connection()
{
  if(uim_fd < 0) {
    uim_fd = uim_helper_init_client_fd(helper_disconnect_cb);
    if(uim_fd > 0)
      read_tag = gdk_input_add(uim_fd, (GdkInputCondition)GDK_INPUT_READ,
                               uim_applet_fd_read_cb, 0);    
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
static void update_plugin() {
  gchar *label = NULL;

  label = g_strdup_printf( "UIM: %s", status_text );

  decal->x_off = 0;
  gkrellm_draw_decal_text( panel, decal, label, -1 );
  gkrellm_draw_panel_layers( panel );

  g_free( label );
}

/*
 * taken from gkrellm2-demos/demo2.c, modified for GKrellUIM
 */
static void create_plugin( GtkWidget *vbox, gint first_create ) {
  GkrellmStyle     *style;
  GkrellmTextstyle *text_style;

  if( first_create ) {
    panel = gkrellm_panel_new0();
  }

  style = gkrellm_meter_style( style_id );

  // GKrellUIM
  text_style = gkrellm_meter_textstyle( style_id );
  decal = gkrellm_create_decal_text( panel, "Aq", text_style, style,
                                     -1, -1, -1);
  gkrellm_panel_configure( panel, NULL, style );

  gkrellm_panel_create( vbox, monitor, panel );
  if ( first_create ) {
    g_signal_connect(G_OBJECT (panel->drawing_area), "expose_event",
                     G_CALLBACK(panel_expose_event), NULL);
  }

  if ( first_create ) {
    // taken from helper-toolbar-common-gtk.c
    uim_fd = -1;
    check_helper_connection();
    uim_helper_client_get_prop_list();
  }
}

/*
 * taken from alltraxclock.c, modified for GKrellUIM
 */
static void create_plugin_tab( GtkWidget *tab_vbox ) {
  GtkWidget *tabs, *text;
  GtkWidget *vbox;
  gchar     *plugin_about_text;

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos( GTK_NOTEBOOK( tabs ), GTK_POS_TOP );
  gtk_box_pack_start( GTK_BOX( tab_vbox ), tabs, TRUE, TRUE, 0 );

  plugin_about_text = g_strdup(
	"GKrellUIM "PACKAGE_VERSION"\n"
	"GKrellM UIM helper Plugin\n\n"
	"Copyright (C) 2004 dai@kip\n"
	"dai@kip.iis.toyama-u.ac.jp\n"
	"http://www.kip.iis.toyama-u.ac.jp/~dai/\n\n"
	"Released under the GNU General Public License\n" );
//	"GKrellUIM comes with ABSOLUTELY NO WARRANTY\n" );
  vbox = gtk_label_new( "About" );
  text = gtk_label_new( plugin_about_text );
  gtk_notebook_append_page( GTK_NOTEBOOK( tabs ), text, vbox );
  g_free( plugin_about_text );
}

/*
 * plugin_gkrelluim
 */
static GkrellmMonitor plugin_gkrelluim = {
  "GKrellUIM",	/* Name, for config tab.	*/
  0,		/* ID, 0 if a plugin		*/
  create_plugin,	/* The create_plugin() function */
  update_plugin,	/* The update_plugin() function */
  create_plugin_tab,	/* The create_plugin_tab() config function	*/
  NULL,		/* The apply_plugin_config() function		*/

  NULL,		/* The save_plugin_config() function	*/
  NULL,		/* The load_plugin_config() function	*/
  "gkrelluim",	/* config keyword			*/

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
  style_id = gkrellm_add_meter_style( &plugin_gkrelluim, "gkrelluim" );
  monitor = &plugin_gkrelluim;

  return &plugin_gkrelluim;
}
