/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
 *
 *  ChmSee is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.

 *  ChmSee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with ChmSee; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

/***************************************************************************
 *   Copyright (C) 2003 by zhong                                           *
 *   zhongz@163.com                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtk/gtk.h>

#include "startup.h"

static void
startup_popup_cb(gpointer data)
{
        gtk_widget_destroy(GTK_WIDGET (data));
}

void 
startup_popup_new(void)
{
        GtkWidget *window;
        GdkPixbuf *pixbuf;
        GdkPixmap *pixmap;
        gint w, h;

        /* following code stolen from StarDict <http://stardict.sourceforge.net> */
        gtk_window_set_auto_startup_notification(FALSE);

        pixbuf = gdk_pixbuf_new_from_file(CHMSEE_DATA_DIR "/chmsee-splash.png", NULL);

        w = gdk_pixbuf_get_width(pixbuf);
        h = gdk_pixbuf_get_height(pixbuf);
        
        window = gtk_window_new(GTK_WINDOW_POPUP);

        gtk_widget_set_app_paintable(window, TRUE);
        gtk_window_set_title(GTK_WINDOW (window), "ChmSee");
        gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);
        gtk_widget_set_size_request(window, w, h);
        gtk_widget_show(window);
        
        pixmap = gdk_pixmap_new(window->window, w, h, -1);
        gdk_pixbuf_render_to_drawable(pixbuf, pixmap,
                                      window->style->fg_gc[GTK_STATE_NORMAL],
                                      0, 0, 0, 0, w,
                                      h, GDK_RGB_DITHER_NORMAL, 0, 0);
        
        gdk_window_set_back_pixmap(window->window, pixmap, FALSE);
        gdk_window_clear(window->window);
        g_object_unref(pixbuf);
        g_object_unref(pixmap);
  
        while (gtk_events_pending())
                gtk_main_iteration();
  
        gtk_init_add((GtkFunction)startup_popup_cb, (gpointer)window);
        
        gtk_window_set_auto_startup_notification(TRUE);
}
