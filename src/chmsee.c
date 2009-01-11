/*
 *  Copyright (C) 2006 Ji YongGang <jungle@soforge-studio.com>
 *  Copyright (C) 2009 LI Daobing <lidaobing@gmail.com>
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

#include "chmsee.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <unistd.h>             /* R_OK */

#include "html.h"
#include "booktree.h"
#include "ui_bookmarks.h"
#include "setup.h"
#include "link.h"
#include "utils.h"
#include "gecko_utils.h"

static void chmsee_class_init(ChmSeeClass *);
static void chmsee_init(ChmSee *);
static void chmsee_finalize(GObject *);

static gboolean delete_cb(GtkWidget *, GdkEvent *, ChmSee *);
static void destroy_cb(GtkWidget *, ChmSee *);
static gboolean configure_event_cb(GtkWidget *, GdkEventConfigure *, ChmSee *);

static gboolean keypress_event_cb(GtkWidget *, GdkEventKey *, ChmSee *);
static void open_response_cb(GtkWidget *, gint, ChmSee *);
static void about_response_cb(GtkDialog *, gint, gpointer);
static void booktree_link_selected_cb(GObject *, Link *, ChmSee *);
static void bookmarks_link_selected_cb(GObject *, Link *, ChmSee *);
static void control_switch_page_cb(GtkNotebook *, GtkNotebookPage *, guint , ChmSee *);
static void html_switch_page_cb(GtkNotebook *, GtkNotebookPage *, guint , ChmSee *);
static void html_location_changed_cb(Html *, const gchar *, ChmSee *);
static gboolean html_open_uri_cb(Html *, const gchar *, ChmSee *);
static void html_title_changed_cb(Html *, const gchar *, ChmSee *);
static void html_context_normal_cb(Html *, ChmSee *);
static void html_context_link_cb(Html *, const gchar *, ChmSee *);
static void html_open_new_tab_cb(Html *, const gchar *, ChmSee *);
static void html_link_message_cb(Html *, const gchar *, ChmSee *);

static void on_open(GtkWidget *, ChmSee *);
static void on_close_tab(GtkWidget *, ChmSee *);
static void on_setup(GtkWidget *, ChmSee *);
static void on_copy(GtkWidget *, ChmSee *);
static void on_copy_page_location(GtkWidget*, ChmSee*);
static void on_select_all(GtkWidget *, ChmSee *);
static void on_back(GtkWidget *, ChmSee *);
static void on_forward(GtkWidget *, ChmSee *);
static void on_home(GtkWidget *, ChmSee *);
static void on_zoom_in(GtkWidget *, ChmSee *);
static void on_zoom_reset(GtkWidget *, ChmSee *);
static void on_zoom_out(GtkWidget *, ChmSee *);
static void on_about(GtkWidget *);
static void on_open_new_tab(GtkWidget *, ChmSee *);
static void on_close_current_tab(GtkWidget *, ChmSee *);
static void on_context_new_tab(GtkWidget *, ChmSee *);
static void on_context_copy_link(GtkWidget *, ChmSee *);

static void chmsee_quit(ChmSee *);
static GtkWidget *get_widget(ChmSee *, gchar *);
static void window_populate(ChmSee *);
static void display_book(ChmSee *, ChmFile *);
static void close_current_book(ChmSee *);
static void new_tab(ChmSee *, const gchar *);
static Html *get_active_html(ChmSee *);
static void check_history(ChmSee *, Html *);
static void update_tab_title(ChmSee *, Html *);
static void tab_set_title(ChmSee *, Html *, const gchar *);
static void open_homepage(ChmSee *);
static void reload_current_page(ChmSee *);
static void update_status_bar(ChmSee *, const gchar *);

static gchar *context_menu_link = NULL;
static GtkWindowClass *parent_class = NULL;

GType
chmsee_get_type(void)
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info = {
                        sizeof(ChmSeeClass),
                        NULL,
                        NULL,
                        (GClassInitFunc)chmsee_class_init,
                        NULL,
                        NULL,
                        sizeof(ChmSee),
                        0,
                        (GInstanceInitFunc)chmsee_init,
                };

                type = g_type_register_static(GTK_TYPE_WINDOW,
                                              "ChmSee",
                                              &info, 0);
        }

        return type;
}

static void
chmsee_class_init(ChmSeeClass *klass)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent(klass);

        object_class = G_OBJECT_CLASS(klass);

        object_class->finalize = chmsee_finalize;
}

static void
chmsee_init(ChmSee *chmsee)
{
        chmsee->home = g_build_filename(g_get_home_dir(), ".chmsee", NULL);

        d(g_debug("chmsee home = %s", chmsee->home));

        if (!g_file_test(chmsee->home, G_FILE_TEST_IS_DIR))
                mkdir(chmsee->home, 0777);

        chmsee->cache_dir = g_build_filename(chmsee->home, "bookshelf", NULL);

        if (!g_file_test(chmsee->cache_dir, G_FILE_TEST_IS_DIR))
                mkdir(chmsee->cache_dir, 0777);

        chmsee->lang = 0;
        chmsee->last_dir = g_strdup(g_get_home_dir());

        chmsee->book = NULL;
        chmsee->html_notebook = NULL;
        chmsee->pos_x = -100;
        chmsee->pos_y = -100;
        chmsee->width = 0;
        chmsee->height = 0;
        chmsee->has_toc = FALSE;
        chmsee->has_index = FALSE;

        g_signal_connect(G_OBJECT (chmsee),
                         "key-press-event",
                         G_CALLBACK (keypress_event_cb),
                         chmsee);
}

static void
chmsee_finalize(GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* callbacks */

static gboolean
delete_cb(GtkWidget *widget, GdkEvent *event, ChmSee *chmsee)
{
        d(g_message("window delete"));
        return FALSE;
}

static void
destroy_cb(GtkWidget *widget, ChmSee *chmsee)
{
        chmsee_quit(chmsee);
}

static gboolean
configure_event_cb(GtkWidget *widget, GdkEventConfigure *event, ChmSee *chmsee)
{
        if (chmsee->html_notebook != NULL 
            && (event->width != chmsee->width || event->height != chmsee->height))
                reload_current_page(chmsee);

        chmsee->width = event->width;
        chmsee->height = event->height;
        chmsee->pos_x = event->x;
        chmsee->pos_y = event->y;

        return FALSE;
}

static gboolean
keypress_event_cb(GtkWidget *widget, GdkEventKey *event, ChmSee *chmsee)
{
        if (event->keyval == GDK_Escape) {
                gtk_window_iconify(GTK_WINDOW (chmsee));
                return TRUE;
        }

        return FALSE;
}

static void
open_response_cb(GtkWidget *widget, gint response_id, ChmSee *chmsee)
{
        gchar *filename = NULL;

        if (response_id == GTK_RESPONSE_OK)
                filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (widget));

        gtk_widget_destroy(widget);

        if (filename != NULL)
                chmsee_open_file(chmsee, filename);

        g_free(filename);
}

static void
booktree_link_selected_cb(GObject *ignored, Link *link, ChmSee *chmsee)
{
        Html *html;

        d(g_debug("booktree link selected: %s", link->uri));
        if (!g_ascii_strcasecmp(CHMSEE_NO_LINK, link->uri))
                return;

        html = get_active_html(chmsee);

        g_signal_handlers_block_by_func(html, html_open_uri_cb, chmsee);

        html_open_uri(html, g_build_filename(chmsee->book->dir, link->uri, NULL));

        g_signal_handlers_unblock_by_func(html, html_open_uri_cb, chmsee);

        check_history(chmsee, html);
}

static void
bookmarks_link_selected_cb(GObject *ignored, Link *link, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_open_uri(html, link->uri);
        check_history(chmsee, html);
}

static void
control_switch_page_cb(GtkNotebook *notebook, GtkNotebookPage *page, guint new_page_num, ChmSee *chmsee)
{
        d(g_debug("switch page : current page = %d", gtk_notebook_get_current_page(notebook)));
}

static void
html_switch_page_cb(GtkNotebook *notebook, GtkNotebookPage *page, guint new_page_num, ChmSee *chmsee)
{
  GtkWidget *new_page;

  new_page = gtk_notebook_get_nth_page(notebook, new_page_num);

  if (new_page) {
    Html *new_html;
    gchar *title, *location;

    new_html = g_object_get_data(G_OBJECT (new_page), "html");

    update_tab_title(chmsee, new_html);

    title = html_get_title(new_html);
    location = html_get_location(new_html);

    if (location != NULL && strlen(location)) {
      if (strlen(title)) {
        ui_bookmarks_set_current_link(UIBOOKMARKS (chmsee->bookmark_tree), title, location);
      } else {
        const gchar *book_title;
                        
        book_title = booktree_get_selected_book_title(BOOKTREE (chmsee->booktree));
        ui_bookmarks_set_current_link(UIBOOKMARKS (chmsee->bookmark_tree), book_title, location);
      }

      /* Sync the book tree. */
      if (chmsee->has_toc)
        booktree_select_uri(BOOKTREE (chmsee->booktree), location);
    }

    check_history(chmsee, new_html);
  } else {
    gtk_window_set_title(GTK_WINDOW (chmsee), "ChmSee");
    check_history(chmsee, NULL);
  }
}

static void
html_location_changed_cb(Html *html, const gchar *location, ChmSee *chmsee)
{
        d(g_debug("html location changed cb: %s", location));

        if (html == get_active_html(chmsee))
                check_history(chmsee, html);
}

static gboolean
html_open_uri_cb(Html *html, const gchar *uri, ChmSee *chmsee)
{
  static const char* prefix = "file://";
  static int prefix_len = 7;
  
  d(g_message("html open uri cb"));

  if(g_str_has_prefix(uri, prefix)) {
    if(g_access(uri+prefix_len, R_OK) < 0) {
      gchar* newfname = correct_filename(uri+prefix_len);
      if(newfname) {
        g_message(_("URI redirect: \"%s\" -> \"%s\""), uri, newfname);
        html_open_uri(html, newfname);
        free(newfname);
        return TRUE;
      }
    }
  }

  if ((html == get_active_html(chmsee)) && chmsee->has_toc)
    booktree_select_uri(BOOKTREE (chmsee->booktree), uri);

  return FALSE;
}

static void
html_title_changed_cb(Html *html, const gchar *title, ChmSee *chmsee)
{
        gchar *location;

        d(g_debug("html title changed cb %s", title));

        update_tab_title(chmsee, get_active_html(chmsee));

        location = html_get_location(html);

        if (location != NULL && strlen(location)) {
                if (strlen(title))
                        ui_bookmarks_set_current_link(UIBOOKMARKS (chmsee->bookmark_tree), title, location);
                else {
                        const gchar *book_title;
                        
                        book_title = booktree_get_selected_book_title(BOOKTREE (chmsee->booktree));
                        ui_bookmarks_set_current_link(UIBOOKMARKS (chmsee->bookmark_tree), book_title, location);
                }
        }
}

/* Popup html context menu */
static void
html_context_normal_cb(Html *html, ChmSee *chmsee)
{
        GladeXML *glade;
        GtkWidget *menu;
        GtkWidget *menu_item;

        gboolean back_state, forward_state;

        d(g_message("html context-normal event"));

        back_state = html_can_go_back(html);
        forward_state = html_can_go_forward(html);

        glade = glade_xml_new(CHMSEE_DATA_DIR"/"GLADE_FILE, "html_context_normal", NULL);
        menu = glade_xml_get_widget(glade, "html_context_normal");

        menu_item = glade_xml_get_widget(glade, "menu_back");
        g_signal_connect(G_OBJECT (menu_item), 
                         "activate",
                         G_CALLBACK (on_back),
                         chmsee);
        gtk_widget_set_sensitive(menu_item, back_state);

        menu_item = glade_xml_get_widget(glade, "menu_forward");
        g_signal_connect(G_OBJECT (menu_item), 
                         "activate",
                         G_CALLBACK (on_forward),
                         chmsee);
        gtk_widget_set_sensitive(menu_item, forward_state);

        menu_item = glade_xml_get_widget(glade, "menu_copy");
        g_signal_connect(G_OBJECT (menu_item), 
                         "activate",
                         G_CALLBACK (on_copy),
                         chmsee);

        menu_item = glade_xml_get_widget(glade, "menu_select_all");
        g_signal_connect(G_OBJECT (menu_item), 
                         "activate",
                         G_CALLBACK (on_select_all),
                         chmsee);

        g_signal_connect(G_OBJECT(glade_xml_get_widget(glade, "menu_copy_page_location")),
                         "activate",
                         G_CALLBACK(on_copy_page_location),
                         chmsee);

        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, GDK_CURRENT_TIME);

	g_object_unref(glade);
}

/* Popup html context menu when mouse over hyper link */
static void
html_context_link_cb(Html *html, const gchar *link, ChmSee *chmsee)
{
        GladeXML *glade;
        GtkWidget *menu;
        GtkWidget *menu_item;

        d(g_debug("html context-link event: %s", link));

        g_free(context_menu_link);

        context_menu_link = g_strdup(link);

        glade = glade_xml_new(CHMSEE_DATA_DIR"/"GLADE_FILE, "html_context_link", NULL);
        menu = glade_xml_get_widget(glade, "html_context_link");

        menu_item = glade_xml_get_widget(glade, "menu_new_tab");
        g_signal_connect(G_OBJECT (menu_item), 
                         "activate",
                         G_CALLBACK (on_context_new_tab),
                         chmsee);
        if (!g_str_has_prefix(context_menu_link, "file://"))
                gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = glade_xml_get_widget(glade, "menu_copy_link");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_context_copy_link),
                         chmsee);

        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, GDK_CURRENT_TIME);

	g_object_unref(glade);
}

static void
html_open_new_tab_cb(Html *html, const gchar *location, ChmSee *chmsee)
{
        d(g_debug("html open new tab callback: %s", location));

        new_tab(chmsee, location);
}

static void
html_link_message_cb(Html *html, const gchar *url, ChmSee *chmsee)
{
        update_status_bar(chmsee, url);
}

/* Toolbar button events */

static void
on_open(GtkWidget *widget, ChmSee *chmsee)
{
        GladeXML *glade;
        GtkWidget *dialog;
        GtkFileFilter *filter;

        /* create openfile dialog */
        glade = glade_xml_new(CHMSEE_DATA_DIR"/"GLADE_FILE, "openfile_dialog", NULL);
        dialog = glade_xml_get_widget(glade, "openfile_dialog");

        g_signal_connect(G_OBJECT (dialog),
                         "response",
                         GTK_SIGNAL_FUNC (open_response_cb),
                         chmsee);

        /* File list fiter */
        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, _("CHM Files"));
        gtk_file_filter_add_pattern(filter, "*.[cC][hH][mM]");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filter);

        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, _("All Files"));
        gtk_file_filter_add_pattern(filter, "*");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filter);

        /* Previous opened folder */
        if (chmsee->last_dir)
                gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), chmsee->last_dir);

	g_object_unref(glade);
}

static void
on_close_tab(GtkWidget *widget, ChmSee *chmsee)
{
        gint num_pages, number, i;
        GtkWidget *tab_label, *page;

        number = -1;
        num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK (chmsee->html_notebook));

        if (num_pages == 1) {
                chmsee_quit(chmsee);

                return;
        }

        for (i = 0; i < num_pages; i++) {
                GList *children, *l;
                
                d(g_debug("page %d", i));
                page = gtk_notebook_get_nth_page(GTK_NOTEBOOK (chmsee->html_notebook), i);

                tab_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK (chmsee->html_notebook), page);
                d(g_message("tab_label"));
                children = gtk_container_get_children(GTK_CONTAINER (tab_label));

                for (l = children; l; l = l->next) {
                        if (widget == l->data) {
                                d(g_debug("found tab on page %d", i));
                                number = i;
                                break;
                        }
                }

                if (number >= 0) {
                        gtk_notebook_remove_page(GTK_NOTEBOOK (chmsee->html_notebook), number);

                        break;
                }
        }
}

static void
on_copy(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        d(g_message("On Copy"));

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        html_copy_selection(html);
}

static void
on_copy_page_location(GtkWidget* widget, ChmSee* chmsee) {
  Html* html = get_active_html(chmsee);
  gchar* location = html_get_location(html);
  if(!location) return;
  
  gtk_clipboard_set_text(
    gtk_clipboard_get(GDK_SELECTION_PRIMARY),
    location,
    -1);
  gtk_clipboard_set_text(
    gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
    location,
    -1);
}

static void
on_select_all(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        d(g_message("On Select All"));

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        html_select_all(html);
}

static void
on_setup(GtkWidget *widget, ChmSee *chmsee)
{
        setup_window_new(chmsee);
}

static void 
on_back(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_go_back(html);
}

static void 
on_forward(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_go_forward(html);
}

static void
on_home(GtkWidget *widget, ChmSee *chmsee)
{
        if (chmsee->book->home != NULL)
                open_homepage(chmsee);
}

static void
on_zoom_in(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_increase_size(html);
}

static void
on_zoom_reset(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_reset_size(html);
}

static void
on_zoom_out(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);
        html_decrease_size(html);
}

static void
about_response_cb(GtkDialog *dialog, gint response_id, gpointer user_data)
{
        if (response_id == GTK_RESPONSE_CANCEL)
                gtk_widget_destroy(GTK_WIDGET (dialog));
}

static void 
on_about(GtkWidget *widget)
{
        GladeXML *glade;
        GtkWidget *dialog;

        glade = glade_xml_new(CHMSEE_DATA_DIR"/"GLADE_FILE, "about_dialog", NULL);
        dialog = glade_xml_get_widget(glade, "about_dialog");

        g_signal_connect(G_OBJECT (dialog), 
                         "response",
                         G_CALLBACK (about_response_cb),
                         NULL);

        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG (dialog), PACKAGE_VERSION);

	g_object_unref(glade);
}

static void
hpanes_toggled_cb(GtkToggleToolButton *widget, ChmSee *chmsee)
{
        gboolean state;
        GtkWidget *icon_widget;
        GtkWidget *control_vbox;

        g_object_get(widget, "active", &state, NULL);
        d(g_debug("hpanes_toggled: %d", state));

        control_vbox = get_widget(chmsee, "control_vbox");

        if (state) {
                icon_widget = gtk_image_new_from_file(CHMSEE_DATA_DIR"/hide-pane.png");
                gtk_widget_show(control_vbox);
        } else {
                icon_widget = gtk_image_new_from_file(CHMSEE_DATA_DIR"/show-pane.png");
                gtk_widget_hide(control_vbox);
        }

        reload_current_page(chmsee);
        gtk_widget_show(icon_widget);
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON (widget), icon_widget);
}

static void
on_open_new_tab(GtkWidget *widget, ChmSee *chmsee)
{
        Html *html;
        gchar *location;

        d(g_message("Open new tab"));

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        location = html_get_location(html);

        if (location != NULL)
                new_tab(chmsee, location);

        g_free(location);
}

static void
on_close_current_tab(GtkWidget *widget, ChmSee *chmsee)
{
        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        if (gtk_notebook_get_n_pages(GTK_NOTEBOOK (chmsee->html_notebook)) == 1)
                return chmsee_quit(chmsee);

        gint page_num;

        page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK (chmsee->html_notebook));

        if (page_num >= 0)
                gtk_notebook_remove_page(GTK_NOTEBOOK (chmsee->html_notebook), page_num);
}

static void
on_context_new_tab(GtkWidget *widget, ChmSee *chmsee)
{
        d(g_debug("On context open new tab: %s", context_menu_link));

        if (context_menu_link != NULL)
                new_tab(chmsee, context_menu_link);
}

static void
on_context_copy_link(GtkWidget *widget, ChmSee *chmsee)
{
        d(g_debug("On context copy link: %s", context_menu_link));

        if (context_menu_link != NULL) {
                gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), 
                                       context_menu_link, -1);
                gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                                       context_menu_link, -1);
        }
}


/* internal functions */

static void
chmsee_quit(ChmSee *chmsee)
{
        if (chmsee->book) {
                save_bookmarks(chmsee->book->dir, 
                               ui_bookmarks_get_list(UIBOOKMARKS (chmsee->bookmark_tree)));
                save_fileinfo(chmsee->book);
                g_object_unref(chmsee->book);
        }

        save_chmsee_config(chmsee);

        g_free(chmsee->home);
        g_free(chmsee->cache_dir);
        g_free(chmsee->last_dir);
        g_free(context_menu_link);
	gecko_utils_shutdown();

        gtk_main_quit();
        exit(0);

        d(g_message("chmsee quit"));
}

static GtkWidget *
get_widget(ChmSee *chmsee, gchar *widget_name)
{
        GladeXML *glade;
        GtkWidget *widget;

        glade = g_object_get_data(G_OBJECT (chmsee), "glade");

        widget = GTK_WIDGET (glade_xml_get_widget(glade, widget_name));
        
        return widget;
}

static void
window_populate(ChmSee *chmsee)
{
        GladeXML *glade;

        glade = glade_xml_new(CHMSEE_DATA_DIR"/"GLADE_FILE, "main_vbox", NULL);

        if (glade == NULL) {
                g_error("Cannot find glade file!");
                exit(1);
        }

        g_object_set_data(G_OBJECT (chmsee), "glade", glade);

        GtkWidget *main_vbox;
        main_vbox = get_widget(chmsee, "main_vbox");
        gtk_container_add(GTK_CONTAINER (chmsee), main_vbox);

        GtkAccelGroup *accel_group;

        accel_group = g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
        gtk_window_add_accel_group(GTK_WINDOW (chmsee), accel_group);

        /* menu item */
        GtkWidget *menu_item;

        menu_item = get_widget(chmsee, "menu_open");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_open),
                         chmsee);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_o,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(chmsee, "menu_new_tab");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_open_new_tab),
                         chmsee);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_t,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(chmsee, "menu_close_tab");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_close_current_tab),
                         chmsee);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_w,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(chmsee, "menu_setup");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_setup),
                         chmsee);

        menu_item = get_widget(chmsee, "menu_copy");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_copy),
                         chmsee);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_c,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(chmsee, "menu_quit");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (destroy_cb),
                         chmsee);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_q,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(chmsee, "menu_home");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_home),
                         chmsee);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(chmsee, "menu_back");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_back),
                         chmsee);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(chmsee, "menu_forward");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_forward),
                         chmsee);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(chmsee, "menu_about");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_about),
                         chmsee);

        /* toolbar buttons */
        GtkWidget *toolbar_button;
        GtkWidget *icon_widget;

        toolbar_button = get_widget(chmsee, "toolbar_open");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_open),
                         chmsee);

        toolbar_button = get_widget(chmsee, "toolbar_setup");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_setup),
                         chmsee);

        toolbar_button = get_widget(chmsee, "toolbar_about");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_about),
                         NULL);

        toolbar_button = get_widget(chmsee, "toolbar_hpanes");
        icon_widget = gtk_image_new_from_file(CHMSEE_DATA_DIR"/show-pane.png");
        gtk_widget_show(icon_widget);
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON (toolbar_button), icon_widget);
        g_object_set(toolbar_button, "active", FALSE, NULL);
        gtk_widget_set_sensitive(toolbar_button, FALSE);
        g_signal_connect(G_OBJECT (toolbar_button),
                         "toggled",
                         G_CALLBACK (hpanes_toggled_cb),
                         chmsee);

        toolbar_button = get_widget(chmsee, "toolbar_back");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_back),
                         chmsee);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_Left,
                                   GDK_MOD1_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(chmsee, "toolbar_forward");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_forward),
                         chmsee);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_Right,
                                   GDK_MOD1_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(chmsee, "toolbar_home");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_home),
                         chmsee);

        toolbar_button = get_widget(chmsee, "toolbar_zoom_in");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_in),
                         chmsee);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_plus,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);


        toolbar_button = get_widget(chmsee, "toolbar_zoom_reset");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_reset),
                         chmsee);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_0,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(chmsee, "toolbar_zoom_out");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_out),
                         chmsee);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_minus,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        GtkWidget *control_vbox;

        control_vbox = get_widget(chmsee, "control_vbox");
        gtk_widget_hide(control_vbox);

        /* status bar */
        chmsee->statusbar = glade_xml_get_widget(glade, "statusbar1");
        chmsee->scid_default = gtk_statusbar_get_context_id(GTK_STATUSBAR (chmsee->statusbar),
                                                            "default");
        update_status_bar(chmsee, _("Ready!"));
}

static void
display_book(ChmSee *chmsee, ChmFile *book)
{
        GNode *link_tree;
        GList *bookmarks_list;

        GtkWidget *booktree_sw;
        GtkWidget *control_vbox;

        d(g_message("display book"));

        /* Close currently opened book */
        if (chmsee->book)
                close_current_book(chmsee);

        chmsee->book = book;

        control_vbox = get_widget(chmsee, "control_vbox");

        /* Book contents TreeView widget */
        chmsee->control_notebook = gtk_notebook_new();
        
        gtk_box_pack_start(GTK_BOX (control_vbox), 
                           GTK_WIDGET (chmsee->control_notebook), 
                           TRUE, 
                           TRUE, 
                           2);
	g_signal_connect(G_OBJECT (chmsee->control_notebook),
                         "switch-page",
                         G_CALLBACK (control_switch_page_cb),
                         chmsee);

        /* TOC */
        if (chmsee->book->link_tree != NULL) {
                link_tree = chmsee->book->link_tree;

                booktree_sw = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (booktree_sw),
                                               GTK_POLICY_NEVER,
                                               GTK_POLICY_AUTOMATIC);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (booktree_sw),
                                                    GTK_SHADOW_IN);
                gtk_container_set_border_width(GTK_CONTAINER (booktree_sw), 2);

                chmsee->booktree = GTK_WIDGET (booktree_new(link_tree));

                gtk_container_add(GTK_CONTAINER (booktree_sw), chmsee->booktree);
                gtk_notebook_append_page(GTK_NOTEBOOK (chmsee->control_notebook),
                                         booktree_sw,
                                         gtk_label_new(_("Topics")));

                g_signal_connect(G_OBJECT (chmsee->booktree),
                                 "link-selected",
                                 G_CALLBACK (booktree_link_selected_cb),
                                 chmsee);

                d(g_message("chmsee has toc"));
                chmsee->has_toc = TRUE;
        }

        /* Bookmarks */
        bookmarks_list = chmsee->book->bookmarks_list;
        chmsee->bookmark_tree = GTK_WIDGET (ui_bookmarks_new(bookmarks_list));

        gtk_notebook_append_page(GTK_NOTEBOOK (chmsee->control_notebook),
                                 chmsee->bookmark_tree,
                                 gtk_label_new (_("Bookmarks")));

        g_signal_connect(G_OBJECT (chmsee->bookmark_tree),
                         "link-selected",
                         G_CALLBACK (bookmarks_link_selected_cb),
                         chmsee);

        GtkWidget *hpaned;

        hpaned = get_widget(chmsee, "hpaned1");
        
        /* HTML tabs notebook */
        chmsee->html_notebook = gtk_notebook_new();
        gtk_paned_add2 (GTK_PANED (hpaned), chmsee->html_notebook);

        g_signal_connect(G_OBJECT (chmsee->html_notebook),
                         "switch-page",
                         G_CALLBACK (html_switch_page_cb),
                         chmsee);

        gtk_widget_show_all(hpaned);
        new_tab(chmsee, NULL);

        gtk_notebook_set_current_page(GTK_NOTEBOOK (chmsee->control_notebook),
                                      g_list_length(bookmarks_list) && chmsee->has_toc ? 1 : 0);
        
        /* Toolbar buttons state */
        GtkWidget *toolbar_button;
                
        toolbar_button = get_widget(chmsee, "toolbar_hpanes");
        gtk_widget_set_sensitive(toolbar_button, TRUE);
        g_object_set(toolbar_button, "active", TRUE, NULL);

        toolbar_button = get_widget(chmsee, "toolbar_zoom_in");
        gtk_widget_set_sensitive(toolbar_button, TRUE);
        toolbar_button = get_widget(chmsee, "toolbar_zoom_reset");
        gtk_widget_set_sensitive(toolbar_button, TRUE);
        toolbar_button = get_widget(chmsee, "toolbar_zoom_out");
        gtk_widget_set_sensitive(toolbar_button, TRUE);

        if (chmsee->book->home) {
                GtkWidget *menu_item;

                open_homepage(chmsee);

                menu_item = get_widget(chmsee, "menu_new_tab");
                gtk_widget_set_sensitive(menu_item, TRUE);

                menu_item = get_widget(chmsee, "menu_close_tab");
                gtk_widget_set_sensitive(menu_item, TRUE);

                menu_item = get_widget(chmsee, "menu_home");
                gtk_widget_set_sensitive(menu_item, TRUE);

                toolbar_button = get_widget(chmsee, "toolbar_home");
                gtk_widget_set_sensitive(toolbar_button, TRUE);
        }

        /* Window title */
        gchar *window_title;

        if (chmsee->book->title != NULL && g_strcasecmp(chmsee->book->title, "(null)") != 0 )
                window_title = g_strdup_printf("%s - ChmSee", chmsee->book->title);
        else
                window_title = g_strdup("ChmSee");

        gtk_window_set_title(GTK_WINDOW (chmsee), window_title);
        g_free(window_title);

        gecko_utils_set_font(GECKO_PREF_FONT_VARIABLE, chmsee->book->variable_font);
        gecko_utils_set_font(GECKO_PREF_FONT_FIXED, chmsee->book->fixed_font);
}

static void
close_current_book(ChmSee *chmsee)
{
        save_bookmarks(chmsee->book->dir, ui_bookmarks_get_list(UIBOOKMARKS (chmsee->bookmark_tree)));
        g_object_unref(chmsee->book);
        gtk_widget_destroy(GTK_WIDGET (chmsee->control_notebook));
        gtk_widget_destroy(GTK_WIDGET (chmsee->html_notebook));
}

static GtkWidget*
new_tab_content(ChmSee *chmsee, const gchar *str)
{
        GtkWidget *widget;
        GtkWidget *label;
        GtkWidget *close_button, *close_image;

        widget = gtk_hbox_new(FALSE, 3);

        label = gtk_label_new(str);
        gtk_label_set_ellipsize(GTK_LABEL (label), PANGO_ELLIPSIZE_END);
        gtk_label_set_single_line_mode(GTK_LABEL (label), TRUE);
        gtk_misc_set_alignment(GTK_MISC (label), 0.0, 0.5);
        gtk_misc_set_padding(GTK_MISC (label), 0, 0);
	gtk_box_pack_start(GTK_BOX (widget), label, TRUE, TRUE, 0);
        g_object_set_data(G_OBJECT (widget), "label", label);

        close_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
	close_image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_widget_show(close_image);
	gtk_container_add(GTK_CONTAINER (close_button), close_image);
	g_signal_connect(G_OBJECT (close_button),
                         "clicked",
	                 G_CALLBACK (on_close_tab),
                         chmsee);

        gtk_box_pack_start(GTK_BOX (widget), close_button, FALSE, FALSE, 0);

	gtk_widget_show_all(widget);

        return widget;
}

static void
new_tab(ChmSee *chmsee, const gchar *location)
{
        Html         *html;
        GtkWidget    *frame;
        GtkWidget    *view;
        GtkWidget    *tab_content;
        gint          num;

        d(g_debug("new_tab : %s", location));

        /* Ignore external link */
        if (location != NULL && !g_str_has_prefix(location, "file://"))
                return;

        html = html_new();

        view = html_get_widget(html);
        gtk_widget_show(view);

        frame = gtk_frame_new(NULL);
        gtk_widget_show(frame);

        gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_IN);
        gtk_container_set_border_width(GTK_CONTAINER (frame), 2);
        gtk_container_add(GTK_CONTAINER (frame), view);

        g_object_set_data(G_OBJECT (frame), "html", html);

        /* Custom label widget, with a close button */
        tab_content = new_tab_content(chmsee, _("No Title"));

        g_signal_connect(G_OBJECT (html),
                         "title-changed",
                         G_CALLBACK (html_title_changed_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "open-uri",
                         G_CALLBACK (html_open_uri_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "location-changed",
                         G_CALLBACK (html_location_changed_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "context-normal",
                         G_CALLBACK (html_context_normal_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "context-link",
                         G_CALLBACK (html_context_link_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "open-new-tab",
                         G_CALLBACK (html_open_new_tab_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (html),
                         "link-message",
                         G_CALLBACK (html_link_message_cb),
                         chmsee);

        num = gtk_notebook_append_page(GTK_NOTEBOOK (chmsee->html_notebook),
                                       frame, tab_content);

        gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK (chmsee->html_notebook),
                                           frame,
                                           TRUE, TRUE,
                                           GTK_PACK_START);

        gtk_widget_realize(view);

        if (location != NULL) {
                html_open_uri(html, location);

                if (chmsee->has_toc)
                        booktree_select_uri(BOOKTREE (chmsee->booktree), location);
        } else
                html_clear(html);

        gtk_notebook_set_current_page(GTK_NOTEBOOK (chmsee->html_notebook), num);
}

static void
open_homepage(ChmSee *chmsee)
{
        Html *html;

        html = get_active_html(chmsee);

        g_signal_handlers_block_by_func(html, html_open_uri_cb, chmsee);

        html_open_uri(html, g_build_filename(chmsee->book->dir, chmsee->book->home, NULL));

        g_signal_handlers_unblock_by_func(html, html_open_uri_cb, chmsee);

        if (chmsee->has_toc)
                booktree_select_uri(BOOKTREE (chmsee->booktree), chmsee->book->home);

        check_history(chmsee, html);
}

static void
reload_current_page(ChmSee *chmsee)
{
        Html *html;
        gchar *location;

        d(g_message("Reload current page"));

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        location = html_get_location(html);

        if (location != NULL)
                html_open_uri(html, location);

        g_free(location);
}

static Html *
get_active_html(ChmSee *chmsee)
{
        GtkWidget *frame;
        gint page_num;

        page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK (chmsee->html_notebook));

        if (page_num == -1)
                return NULL;

        frame = gtk_notebook_get_nth_page(GTK_NOTEBOOK (chmsee->html_notebook), page_num);

        return g_object_get_data(G_OBJECT (frame), "html");
}

static void
check_history(ChmSee *chmsee, Html *html)
{
        GtkWidget *menu_item, *toolbar_button;
        gboolean back_state, forward_state;

        back_state = html_can_go_back(html);
        forward_state = html_can_go_forward(html);

        menu_item = get_widget(chmsee, "menu_back");
        gtk_widget_set_sensitive(menu_item, back_state);
        menu_item = get_widget(chmsee, "menu_forward");
        gtk_widget_set_sensitive(menu_item, forward_state);

        toolbar_button = get_widget(chmsee, "toolbar_back");
        gtk_widget_set_sensitive(toolbar_button, back_state);
        toolbar_button = get_widget(chmsee, "toolbar_forward");
        gtk_widget_set_sensitive(toolbar_button, forward_state);
}

static void
update_tab_title(ChmSee *chmsee, Html *html)
{
        gchar *html_title, *tab_title;
        const gchar *book_title;

        html_title = html_get_title(html);
        
        if (chmsee->has_toc)
                book_title = booktree_get_selected_book_title(BOOKTREE (chmsee->booktree));
        else 
                book_title = "";

        if (book_title && book_title[0] != '\0' &&
            html_title && html_title[0] != '\0' &&
            ncase_compare_utf8_string(book_title, html_title))
                tab_title = g_strdup_printf("%s : %s", book_title, html_title);
        else if (book_title && book_title[0] != '\0')
                tab_title = g_strdup(book_title);
        else if (html_title && html_title[0] != '\0')
                tab_title = g_strdup(html_title);
        else
                tab_title = g_strdup("");

        tab_set_title(chmsee, html, tab_title);

        g_free(html_title);
        g_free(tab_title);
}

static void
tab_set_title(ChmSee *chmsee, Html *html, const gchar *title)
{
        GtkWidget *view;
        GtkWidget *page;
        GtkWidget *widget, *label;
        gint num_pages, i;

        view = html_get_widget(html);

        if (title == NULL || title[0] == '\0')
                title = _("No Title");

        num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK (chmsee->html_notebook));

        for (i = 0; i < num_pages; i++) {
                page = gtk_notebook_get_nth_page(GTK_NOTEBOOK (chmsee->html_notebook), i);

                if (gtk_bin_get_child(GTK_BIN (page)) == view) {
                        widget = gtk_notebook_get_tab_label(GTK_NOTEBOOK (chmsee->html_notebook), page);

                        label = g_object_get_data(G_OBJECT (widget), "label");

                        if (label != NULL)
                                gtk_label_set_text(GTK_LABEL (label), title);

                        break;
                }
        }
}

static void
update_status_bar(ChmSee *chmsee, const gchar *message)
{
        gchar *status;

        status = g_strdup_printf(" %s", message);

        gtk_statusbar_pop(GTK_STATUSBAR(chmsee->statusbar), chmsee->scid_default);
        gtk_statusbar_push(GTK_STATUSBAR(chmsee->statusbar), chmsee->scid_default, status);

        g_free(status);
}

/* external functions */

ChmSee *
chmsee_new(void)
{
        ChmSee *chmsee;

        chmsee = g_object_new(TYPE_CHMSEE, NULL);

        /* Quit event handle */
        g_signal_connect(G_OBJECT (chmsee),
                         "delete_event",
                         G_CALLBACK (delete_cb),
                         chmsee);
        g_signal_connect(G_OBJECT (chmsee),
                         "destroy",
                         G_CALLBACK (destroy_cb),
                         chmsee);

        /* Widget size changed event handle */
        g_signal_connect(G_OBJECT (chmsee),
                         "configure-event",
                         G_CALLBACK (configure_event_cb),
                         chmsee);

        load_chmsee_config(chmsee);

        /* Init gecko */
        gecko_utils_init();
        gecko_utils_set_default_lang(chmsee->lang);

        window_populate(chmsee);

        if (chmsee->pos_x >= 0 && chmsee->pos_y >= 0)
                gtk_window_move(GTK_WINDOW (chmsee), chmsee->pos_x, chmsee->pos_y);

        if (chmsee->width > 0 && chmsee->height > 0)
                gtk_window_resize(GTK_WINDOW (chmsee), chmsee->width, chmsee->height);
        else
                gtk_window_resize(GTK_WINDOW (chmsee), 800, 600);

        gtk_window_set_title(GTK_WINDOW (chmsee), "ChmSee");
        gtk_window_set_icon_from_file(GTK_WINDOW (chmsee), CHMSEE_DATA_DIR"/chmsee-icon.png", NULL);

        return chmsee;
}

void
chmsee_open_file(ChmSee *chmsee, const gchar *filename)
{
        ChmFile *book;

        g_return_if_fail(IS_CHMSEE (chmsee));

        /* Extract chm and get file infomation */
        book = chmfile_new(filename);

        if (book) {
                display_book(chmsee, book);

                chmsee->last_dir = g_strdup_printf("%s", g_path_get_dirname(filename));
        } else {
                /* Popup an error message dialog */
                GtkWidget *msg_dialog;
                
                msg_dialog = gtk_message_dialog_new(GTK_WINDOW (chmsee),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE,
                                                    _("Error loading file '%s'"),
                                                    filename);
                gtk_dialog_run(GTK_DIALOG (msg_dialog));
                gtk_widget_destroy(msg_dialog);
        }
}
