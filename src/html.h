/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
 *  Copyright (c) 2014           Xianguang Zhou <xianguang.zhou@outlook.com>
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

#ifndef __HTML_H__
#define __HTML_H__

#include <glib-object.h>
#include <gtk/gtk.h>
//#include <gtkmozembed.h>
#include <webkit/webkit.h>

#define TYPE_HTML \
        (html_get_type())
#define HTML(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_HTML, Html))
#define HTML_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST((k), TYPE_HTML, HtmlClass))
#define IS_HTML(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_HTML))
#define IS_HTML_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_HTML))

typedef struct _Html        Html;
typedef struct _HtmlClass   HtmlClass;

struct _Html {
        GObject parent;
//        GtkMozEmbed *gecko;
        GtkScrolledWindow * scrolled_window;
        WebKitWebView *gecko;
};

struct _HtmlClass {
        GObjectClass parent_class;

        /* Signals */
        void (* title_changed) (Html *html, const gchar *title);
        void (* location_changed) (Html *html, const gchar *location);
        gboolean (* open_uri) (Html *html, const gchar *uri);
        void (* context_normal) (Html *html);
        void (* context_link) (Html *html, const gchar *link);
//        void (* open_new_tab) (Html *html, const gchar *uri);
        Html * (* open_new_tab) (Html *html, WebKitWebFrame *frame);
        void (* link_message) (Html *html, const gchar *link);
        gboolean (* scroll_web_view) (Html *html, GdkEvent *event);
};

GType html_get_type(void);
Html *html_new(void);
void html_clear(Html *);
void html_open_uri(Html *, const gchar *);

GtkWidget *html_get_widget(Html *);
gboolean html_can_go_forward(Html *);
gboolean html_can_go_back(Html *);
void html_go_forward(Html *);
void html_go_back(Html *);
//gchar *html_get_title(Html *);
const gchar *html_get_title(Html *);
//gchar *html_get_location(Html *);
const gchar *html_get_location(Html *);
void html_copy_selection(Html *);
void html_select_all(Html *);
void html_increase_size(Html *);
void html_reset_size(Html *);
void html_decrease_size(Html *);
void html_shutdown(Html*);
void html_set_variable_font(Html*, const gchar*);
void html_set_fixed_font(Html*, const gchar*);
void html_init_system(void);
void html_set_default_lang(gint);

#endif /* !__HTML_H__ */
