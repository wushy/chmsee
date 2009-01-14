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

#include "html.h"

#include <string.h>

#include "marshal.h"
#include "utils.h"
#include "gecko_utils.h"

static void html_class_init(HtmlClass *);
static void html_init(Html *);

static void html_title_cb(GtkMozEmbed *, Html *);
static void html_location_cb(GtkMozEmbed *, Html *);
static gboolean html_open_uri_cb(GtkMozEmbed *, const gchar *, Html *);
static gboolean html_mouse_click_cb(GtkMozEmbed *, gpointer, Html *);
static gboolean html_link_message_cb(GtkMozEmbed *, Html *);
static void html_child_add_cb(GtkMozEmbed *, GtkWidget *, Html *);
static void html_child_remove_cb(GtkMozEmbed *, GtkWidget *, Html *);
static void html_child_grab_focus_cb(GtkWidget *, Html *);

/* Signals */
enum {
        TITLE_CHANGED,
        LOCATION_CHANGED,
        OPEN_URI,
        CONTEXT_NORMAL,
        CONTEXT_LINK,
        OPEN_NEW_TAB,
        LINK_MESSAGE,
        LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = { 0 };

/* Has the value of the URL under the mouse pointer, otherwise NULL */
static gchar *current_url = NULL;

GType
html_get_type (void)
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info =
                        {
                                sizeof(HtmlClass),
                                NULL,
                                NULL,
                                (GClassInitFunc)html_class_init,
                                NULL,
                                NULL,
                                sizeof(Html),
                                0,
                                (GInstanceInitFunc)html_init,
                        };
                
                type = g_type_register_static(G_TYPE_OBJECT,
                                              "Html", 
                                              &info, 0);
        }
        
        return type;
}

static void
html_class_init(HtmlClass *klass)
{
        signals[TITLE_CHANGED] = 
                g_signal_new ("title-changed",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);

        signals[LOCATION_CHANGED] =
                g_signal_new("location-changed",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

        signals[OPEN_URI] =
                g_signal_new("open-uri",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_BOOLEAN__STRING,
                             G_TYPE_BOOLEAN,
                             1, G_TYPE_STRING);

        signals[CONTEXT_NORMAL] =
                g_signal_new("context-normal",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__VOID,
                             G_TYPE_NONE,
                             0);

        signals[CONTEXT_LINK] =
                g_signal_new("context-link",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

        signals[OPEN_NEW_TAB] =
                g_signal_new("open-new-tab",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

        signals[LINK_MESSAGE] = 
                g_signal_new("link-message",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);
}

static void
html_init(Html *html)
{
        html->gecko = GTK_MOZ_EMBED(gtk_moz_embed_new());
        gtk_drag_dest_unset(GTK_WIDGET(html->gecko));

        g_signal_connect(G_OBJECT (html->gecko),
                         "title",
                         G_CALLBACK (html_title_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko), 
                         "location",
                         G_CALLBACK (html_location_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko), 
                         "open-uri",
                         G_CALLBACK (html_open_uri_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko),
                         "dom_mouse_click",
                         G_CALLBACK (html_mouse_click_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko), 
                         "link_message",
                         G_CALLBACK (html_link_message_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko), 
                         "add",
                         G_CALLBACK (html_child_add_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko), 
                         "remove",
                         G_CALLBACK (html_child_remove_cb),
                         html);
}

/* callbacks */

static void
html_title_cb(GtkMozEmbed *embed, Html *html)
{
        char *new_title;

        new_title = gtk_moz_embed_get_title(embed);
        g_signal_emit(html, signals[TITLE_CHANGED], 0, new_title);
        g_free(new_title);
}

static void
html_location_cb(GtkMozEmbed *embed, Html *html)
{
        char *location;

        location = gtk_moz_embed_get_location(embed);
        g_signal_emit(html, signals[LOCATION_CHANGED], 0, location);
        g_free(location);
}

static gboolean
html_open_uri_cb(GtkMozEmbed *embed, const gchar *uri, Html *html)
{
        gboolean ret_val;

        ret_val = TRUE;

        g_signal_emit(html, signals[OPEN_URI], 0, uri, &ret_val);

        /* Reset current url */
        if (current_url != NULL) {
                g_free(current_url);
                current_url = NULL;

                g_signal_emit(html, signals[LINK_MESSAGE], 0, "");
        }

        return ret_val;
}

static gboolean
html_mouse_click_cb(GtkMozEmbed *widget, gpointer dom_event, Html *html)
{
        gint button;
        gint mask;
        
        button = gecko_utils_get_mouse_event_button(dom_event);
        mask = gecko_utils_get_mouse_event_modifiers(dom_event);

        if (button == 2 || (button == 1 && mask & GDK_CONTROL_MASK)) {
                if (current_url) {
                        g_signal_emit(html, signals[OPEN_NEW_TAB], 0, current_url);

                        return TRUE;
                }
        } else if (button == 3) {
                if (current_url)
                        g_signal_emit(html, signals[CONTEXT_LINK], 0, current_url);
                else
                        g_signal_emit(html, signals[CONTEXT_NORMAL], 0);

                return TRUE;
        }

        return FALSE;
}

static gboolean
html_link_message_cb(GtkMozEmbed *widget, Html *html)
{
        g_free(current_url);

        current_url = gtk_moz_embed_get_link_message(widget);
        g_signal_emit(html, signals[LINK_MESSAGE], 0, current_url);

        if (current_url[0] == '\0') {
                g_free(current_url);
                current_url = NULL;
        }

        return FALSE;
}

static void
html_child_add_cb(GtkMozEmbed *embed, GtkWidget *child, Html *html)
{
        g_signal_connect(G_OBJECT (child), 
                         "grab-focus",
                         G_CALLBACK (html_child_grab_focus_cb),
                         html);
}

static void
html_child_remove_cb(GtkMozEmbed *embed, GtkWidget *child, Html *html)
{
        g_signal_handlers_disconnect_by_func(child, html_child_grab_focus_cb, html);
}

static void
html_child_grab_focus_cb(GtkWidget *widget, Html *html)
{
        GdkEvent *event;

        event = gtk_get_current_event();

        if (event == NULL)
                g_signal_stop_emission_by_name(widget, "grab-focus");
        else
                gdk_event_free(event);
}

/* external functions */

Html *
html_new(void)
{
        Html *html;

        html = g_object_new(TYPE_HTML, NULL);

        return html;
}

void
html_clear(Html *html)
{
        static const char *data = "<html><body bgcolor=\"white\"></body></html>";
        
        g_return_if_fail(IS_HTML (html));

        gtk_moz_embed_render_data(html->gecko, data, strlen(data), "file:///", "text/html");
}

void
html_open_uri(Html *html, const gchar *str_uri)
{
        gchar *full_uri;
        
        g_return_if_fail(IS_HTML (html));
        g_return_if_fail(str_uri != NULL);

        if (str_uri[0] == '/')
                full_uri = g_strdup_printf("file://%s", str_uri);
        else
                full_uri = g_strdup(str_uri);
        
        d(g_debug("Open uri %s", full_uri));
        gtk_moz_embed_load_url(html->gecko, full_uri);
        g_free(full_uri);
}

GtkWidget *
html_get_widget(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

        return GTK_WIDGET (html->gecko);
}

gboolean
html_can_go_forward(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), FALSE);

        return gtk_moz_embed_can_go_forward(html->gecko);
}

gboolean
html_can_go_back(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), FALSE);

        return gtk_moz_embed_can_go_back(html->gecko);
}

void
html_go_forward(Html *html)
{
        g_return_if_fail(IS_HTML (html));

        gtk_moz_embed_go_forward(html->gecko);
}

void
html_go_back(Html *html)
{
        g_return_if_fail(IS_HTML (html));

        gtk_moz_embed_go_back(html->gecko);
}

gchar *
html_get_title(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

        return gtk_moz_embed_get_title(html->gecko);
}

gchar *
html_get_location(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

        return gtk_moz_embed_get_location(html->gecko);
}

void
html_copy_selection(Html *html)
{
        g_return_if_fail(IS_HTML (html));

        gecko_utils_copy_selection(html->gecko);
}

void
html_select_all(Html *html)
{
        g_return_if_fail(IS_HTML (html));

        gecko_utils_select_all(html->gecko);
}

void
html_increase_size(Html *html)
{
        gfloat zoom;

        g_return_if_fail(IS_HTML (html));

        zoom = gecko_utils_get_zoom(html->gecko);
        zoom *= 1.2;

        gecko_utils_set_zoom(html->gecko, zoom);
}

void
html_reset_size(Html *html)
{
        g_return_if_fail(IS_HTML (html));

        gecko_utils_set_zoom(html->gecko, 1.0);
}

void
html_decrease_size(Html *html)
{
        gfloat zoom;

        g_return_if_fail(IS_HTML (html));

        zoom = gecko_utils_get_zoom(html->gecko);
        zoom /= 1.2;

        gecko_utils_set_zoom(html->gecko, zoom);
}

void html_shutdown(Html* html) {
  gecko_utils_shutdown();
}

void html_init_system(void) {
  gecko_utils_init();
}

void html_set_default_lang(gint lang) {
  gecko_utils_set_default_lang(lang);
}

void html_set_variable_font(Html*, const gchar* font) {
  gecko_utils_set_font(GECKO_PREF_FONT_VARIABLE, font);
}

void html_set_fixed_font(Html*, const gchar* font) {
  gecko_utils_set_font(GECKO_PREF_FONT_FIXED, font);
}


