/*
 * Copyright (C) 2008 Rodney Cryderman <rcryderman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libawn/awn-cairo-utils.h>
#include <libawn/awn-applet-simple.h>
#include <libawn/awn-applet-dialog.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <libawn-extras/awn-extras.h>

#include "engine_html.h"
#include "applet.h"
#include "configuration.h"


static gboolean
_show_prefs (GtkWidget *widget, GdkEventButton *event, WebApplet * webapplet)
{
  return TRUE;
}

static void
awn_html_dialog_new (WebApplet *webapplet)
{
  webapplet->mainwindow = awn_applet_dialog_new (webapplet->applet);
  webapplet->box = gtk_vbox_new (FALSE, 1);
  gtk_widget_set_size_request (GTK_WIDGET (webapplet->box), webapplet->width,
                               webapplet->height);
  webapplet->viewer = html_web_view_new ();
  gtk_container_add (GTK_CONTAINER (webapplet->box),webapplet->viewer);
  gtk_container_add (GTK_CONTAINER (webapplet->mainwindow), webapplet->box);
  html_web_view_open (webapplet->viewer, webapplet->uri);
}


static gboolean
_button_clicked_event (GtkWidget      *widget,
                       GdkEventButton *event,
                       WebApplet      *webapplet)
{
  GdkEventButton *event_button;
  event_button = (GdkEventButton *)event;
  if (event->button == 1)
  {
    if (GTK_WIDGET_VISIBLE (webapplet->mainwindow))
    {
      gtk_widget_hide (webapplet->mainwindow);
    }
    else
    {
      gtk_widget_show_all (webapplet->mainwindow);
//      webkit_web_view_open (WEBKIT_WEB_VIEW (webapplet->viewer),
//                            "http://www.musicpd.org/");
    }
  }
  else if (event->button == 3)
  {
    static gboolean done_once = FALSE;
    static GtkWidget *menu;
    static GtkWidget *item;
    if (!done_once)
    {
      menu = gtk_menu_new ();
      item = gtk_menu_item_new_with_label ("Preferences");
      gtk_widget_show (item);
      gtk_menu_set_screen (GTK_MENU (menu), NULL);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      g_signal_connect (G_OBJECT (item), "button-press-event",
                        G_CALLBACK (_show_prefs), webapplet);
      done_once=TRUE;
    }
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                    event_button->button, event_button->time);
  }
  return TRUE;
}

static gboolean
_focus_out_event(GtkWidget *widget, GdkEventButton *event, void * null)
{
//  gtk_widget_hide(webapplet->mainwindow);
  return TRUE;
}


static void
_bloody_thing_has_style (GtkWidget *widget,WebApplet *webapplet)
{
  GdkPixbuf *newicon;

  //init_config(webapplet);

  newicon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                      webapplet->applet_icon_name,
                                      webapplet->applet_icon_height,
                                      0, NULL);
  if (!newicon)
  {
    newicon = gdk_pixbuf_new_from_file_at_size (g_filename_from_utf8 (webapplet->applet_icon_name,
                                                                      -1, NULL, NULL, NULL),
                                                webapplet->applet_icon_height,
                                                webapplet->applet_icon_height,
                                                NULL);
  }
  if (newicon)
  {
    webapplet->icon = newicon;
  }
  if (gdk_pixbuf_get_height (webapplet->icon) != webapplet->applet_icon_height)
  {
    GdkPixbuf *oldpbuf = webapplet->icon;
    webapplet->icon = gdk_pixbuf_scale_simple (oldpbuf,
                                               webapplet->applet_icon_height,
                                               webapplet->applet_icon_height,
                                               GDK_INTERP_HYPER);
    g_object_unref (oldpbuf);
  }
  awn_applet_simple_set_temp_icon (AWN_APPLET_SIMPLE (webapplet->applet),
                                   webapplet->icon);
  g_signal_connect (G_OBJECT (webapplet->applet), "button-press-event",
                    G_CALLBACK (_button_clicked_event), webapplet);
  g_signal_connect (G_OBJECT (webapplet->mainwindow), "focus-out-event",
                    G_CALLBACK (_focus_out_event), webapplet);
}

AwnApplet *
awn_applet_factory_initp (gchar *uid, gint orient, gint height)
{
  GdkPixbuf *icon;
  g_on_error_stack_trace (NULL);
  html_init ();
  WebApplet *webapplet = g_malloc (sizeof (WebApplet));
  init_config (webapplet, uid);
  webapplet->applet = AWN_APPLET (awn_applet_simple_new (uid, orient, height));
  gtk_widget_set_size_request (GTK_WIDGET (webapplet->applet), height, -1);
  icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                   "stock_folder", height - 2, 0, NULL);
  if (!icon)
  {
    icon = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, height - 2, height - 2);
    gdk_pixbuf_fill (icon, 0x11881133);
  }
  webapplet->applet_icon_height = height - 2;
  webapplet->icon = icon;
  webapplet->applet_icon_name = g_strdup ("apple-green");
  awn_applet_simple_set_temp_icon (AWN_APPLET_SIMPLE (webapplet->applet), icon);
  gtk_widget_show_all (GTK_WIDGET (webapplet->applet));
  awn_html_dialog_new (webapplet);
  gtk_window_set_focus_on_map (GTK_WINDOW (webapplet->mainwindow), TRUE);
  g_signal_connect_after (G_OBJECT (webapplet->applet), "realize",
                          G_CALLBACK (_bloody_thing_has_style), webapplet);
  return webapplet->applet;
}
/* vim: set et ts=2 sts=2 sw=2 : */