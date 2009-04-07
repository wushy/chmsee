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

#include "config.h"
#include "chmsee.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <unistd.h>             /* R_OK */

#include <glib.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>

#include "ihtml.h"
#include "html-factory.h"
#include "booktree.h"
#include "ui_bookmarks.h"
#include "setup.h"
#include "link.h"
#include "utils.h"

#include "models/chmfile-factory.h"

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
static void html_location_changed_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static gboolean html_open_uri_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static void html_title_changed_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static void html_context_normal_cb(ChmseeIhtml *, ChmSee *);
static void html_context_link_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static void html_open_new_tab_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static void html_link_message_cb(ChmseeIhtml *, const gchar *, ChmSee *);
static void show_sidepane(ChmSee* self);
static void hide_sidepane(ChmSee* self);
static void set_sidepane_state(ChmSee* self, gboolean state);

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
static void on_fullscreen_toggled(ChmSee* self, GtkWidget* menu);
static void on_sidepane_toggled(ChmSee* self, GtkWidget* menu);
static void on_map(ChmSee* self);
static gboolean on_window_state_event(ChmSee* self, GdkEventWindowState* event);

static void chmsee_quit(ChmSee *);
static void chmsee_open_uri(ChmSee *chmsee, const gchar *uri);
static void chmsee_open_file(ChmSee *chmsee, const gchar *filename);
static GtkWidget *get_widget(ChmSee *, gchar *);
static void populate_window(ChmSee *);
static void display_book(ChmSee *, ChmseeIchmfile *);
static void close_current_book(ChmSee *);
static void new_tab(ChmSee *, const gchar *);
static ChmseeIhtml *get_active_html(ChmSee *);
static void check_history(ChmSee *, ChmseeIhtml *);
static void update_tab_title(ChmSee *, ChmseeIhtml *);
static void tab_set_title(ChmSee *, ChmseeIhtml *, const gchar *);
static void open_homepage(ChmSee *);
static void reload_current_page(ChmSee *);
static void update_status_bar(ChmSee *, const gchar *);
static void
chmsee_drag_data_received (GtkWidget          *widget,
                           GdkDragContext     *context,
                           gint                x,
                           gint                y,
                           GtkSelectionData   *selection_data,
                           guint               info,
                           guint               time);

static gchar *context_menu_link = NULL;
static GtkWindowClass *parent_class = NULL;
static const GtkTargetEntry view_drop_targets[] = {
	{ "text/uri-list", 0, 0 }
};

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
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
        parent_class = g_type_class_peek_parent(klass);

        object_class->finalize = chmsee_finalize;
        widget_class->drag_data_received = chmsee_drag_data_received;
}

static void
chmsee_init(ChmSee *chmsee)
{
        chmsee->home = g_build_filename(g_get_home_dir(), ".chmsee", NULL);

        g_debug("chmsee home = %s", chmsee->home);

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
        chmsee->hpaned_position = -1;
        chmsee->has_toc = FALSE;
        chmsee->has_index = FALSE;
        chmsee->fullscreen = FALSE;

        gtk_widget_add_events(GTK_WIDGET(chmsee),
        		GDK_STRUCTURE_MASK);

        g_signal_connect(G_OBJECT (chmsee),
                         "key-press-event",
                         G_CALLBACK (keypress_event_cb),
                         chmsee);
        g_signal_connect(G_OBJECT(chmsee),
        		"map",
        		G_CALLBACK(on_map),
                        NULL);
        g_signal_connect(G_OBJECT(chmsee),
        		"window-state-event",
        		G_CALLBACK(on_window_state_event),
        		NULL);
        gtk_drag_dest_set (GTK_WIDGET (chmsee),
			   GTK_DEST_DEFAULT_ALL,
			   view_drop_targets,
			   G_N_ELEMENTS (view_drop_targets),
			   GDK_ACTION_COPY);

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
        g_message("window delete");
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
keypress_event_cb(GtkWidget *widget, GdkEventKey *event, ChmSee *self)
{
	if (event->keyval == GDK_Escape) {
		if(self->fullscreen) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(get_widget(self, "menu_fullscreen")),
					FALSE);
		} else {
			gtk_window_iconify(GTK_WINDOW (self));
			return TRUE;
		}
	} else if(event->keyval == GDK_F11) {
		if(self->fullscreen) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(get_widget(self, "menu_fullscreen")),
					FALSE);
		}
	} else if(event->keyval == GDK_F9) {
		if(self->fullscreen) {
			set_sidepane_state(self,
					!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(get_widget(self, "menu_sidepane"))));
		}
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
        ChmseeIhtml* html;

        g_debug("booktree link selected: %s", link->uri);
        if (!g_ascii_strcasecmp(CHMSEE_NO_LINK, link->uri))
                return;

        html = get_active_html(chmsee);

        g_signal_handlers_block_by_func(html, html_open_uri_cb, chmsee);

        chmsee_ihtml_open_uri(html, g_build_filename(
                        chmsee_ichmfile_get_dir(chmsee->book), link->uri, NULL));

        g_signal_handlers_unblock_by_func(html, html_open_uri_cb, chmsee);

        check_history(chmsee, html);
}

static void
bookmarks_link_selected_cb(GObject *ignored, Link *link, ChmSee *chmsee)
{
  chmsee_ihtml_open_uri(get_active_html(chmsee), link->uri);
  check_history(chmsee, get_active_html(chmsee));
}

static void
control_switch_page_cb(GtkNotebook *notebook, GtkNotebookPage *page, guint new_page_num, ChmSee *chmsee)
{
        g_debug("switch page : current page = %d", gtk_notebook_get_current_page(notebook));
}

static void
html_switch_page_cb(GtkNotebook *notebook, GtkNotebookPage *page, guint new_page_num, ChmSee *chmsee)
{
  GtkWidget *new_page;

  new_page = gtk_notebook_get_nth_page(notebook, new_page_num);

  if (new_page) {
    ChmseeIhtml* new_html;
    const gchar* title;
    const gchar* location;

    new_html = g_object_get_data(G_OBJECT (new_page), "html");

    update_tab_title(chmsee, new_html);

    title = chmsee_ihtml_get_title(new_html);
    location = chmsee_ihtml_get_location(new_html);

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
html_location_changed_cb(ChmseeIhtml *html, const gchar *location, ChmSee *chmsee)
{
        g_debug("html location changed cb: %s", location);

        if (html == get_active_html(chmsee))
                check_history(chmsee, html);
}

static gboolean
html_open_uri_cb(ChmseeIhtml* html, const gchar *uri, ChmSee *chmsee)
{
  static const char* prefix = "file://";
  static int prefix_len = 7;

  if(g_str_has_prefix(uri, prefix)) {
    /* FIXME: can't disable the DND function of GtkMozEmbed */
    if(g_str_has_suffix(uri, ".chm")
       || g_str_has_suffix(uri, ".CHM")) {
      chmsee_open_uri(chmsee, uri);
    }

    if(g_access(uri+prefix_len, R_OK) < 0) {
      gchar* newfname = correct_filename(uri+prefix_len);
      if(newfname) {
        g_message(_("URI redirect: \"%s\" -> \"%s\""), uri, newfname);
        chmsee_ihtml_open_uri(html, newfname);
        g_free(newfname);
        return TRUE;
      }
    }
  }

  if ((html == get_active_html(chmsee)) && chmsee->has_toc)
    booktree_select_uri(BOOKTREE (chmsee->booktree), uri);

  return FALSE;
}

static void
html_title_changed_cb(ChmseeIhtml *html, const gchar *title, ChmSee *chmsee)
{
        const gchar *location;

        g_debug("html title changed cb %s", title);

        update_tab_title(chmsee, get_active_html(chmsee));

        location = chmsee_ihtml_get_location(html);

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
html_context_normal_cb(ChmseeIhtml *html, ChmSee *chmsee)
{
        GladeXML *glade;
        GtkWidget *menu;
        GtkWidget *menu_item;

        gboolean back_state, forward_state;

        g_message("html context-normal event");

        back_state = chmsee_ihtml_can_go_back(html);
        forward_state = chmsee_ihtml_can_go_forward(html);

        glade = glade_xml_new(get_resource_path(GLADE_FILE), "html_context_normal", NULL);
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
html_context_link_cb(ChmseeIhtml *html, const gchar *link, ChmSee *chmsee)
{
        GladeXML *glade;
        GtkWidget *menu;
        GtkWidget *menu_item;

        g_debug("html context-link event: %s", link);

        g_free(context_menu_link);

        context_menu_link = g_strdup(link);

        glade = glade_xml_new(get_resource_path(GLADE_FILE), "html_context_link", NULL);
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
html_open_new_tab_cb(ChmseeIhtml *html, const gchar *location, ChmSee *chmsee)
{
        g_debug("html open new tab callback: %s", location);

        new_tab(chmsee, location);
}

static void
html_link_message_cb(ChmseeIhtml *html, const gchar *url, ChmSee *chmsee)
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
        glade = glade_xml_new(get_resource_path(GLADE_FILE), "openfile_dialog", NULL);
        dialog = glade_xml_get_widget(glade, "openfile_dialog");

        g_signal_connect(G_OBJECT (dialog),
                         "response",
                         G_CALLBACK (open_response_cb),
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

                g_debug("page %d", i);
                page = gtk_notebook_get_nth_page(GTK_NOTEBOOK (chmsee->html_notebook), i);

                tab_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK (chmsee->html_notebook), page);
                g_message("tab_label");
                children = gtk_container_get_children(GTK_CONTAINER (tab_label));

                for (l = children; l; l = l->next) {
                        if (widget == l->data) {
                                g_debug("found tab on page %d", i);
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
        g_message("On Copy");

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        chmsee_ihtml_copy_selection(get_active_html(chmsee));
}

static void
on_copy_page_location(GtkWidget* widget, ChmSee* chmsee) {
  ChmseeIhtml* html = get_active_html(chmsee);
  const gchar* location = chmsee_ihtml_get_location(html);
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
        ChmseeIhtml *html;

        g_message("On Select All");

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        chmsee_ihtml_select_all(html);
}

static void
on_setup(GtkWidget *widget, ChmSee *chmsee)
{
        setup_window_new(chmsee);
}

static void
on_back(GtkWidget *widget, ChmSee *chmsee)
{
  chmsee_ihtml_go_back(get_active_html(chmsee));
}

static void
on_forward(GtkWidget *widget, ChmSee *chmsee)
{
  chmsee_ihtml_go_forward(get_active_html(chmsee));
}

static void
on_home(GtkWidget *widget, ChmSee *chmsee)
{
  if (chmsee_ichmfile_get_home(chmsee->book) != NULL) {
    open_homepage(chmsee);
  }
}

static void
on_zoom_in(GtkWidget *widget, ChmSee *chmsee)
{
  chmsee_ihtml_increase_size(get_active_html(chmsee));
}

static void
on_zoom_reset(GtkWidget *widget, ChmSee *chmsee)
{
  chmsee_ihtml_reset_size(get_active_html(chmsee));
}

static void
on_zoom_out(GtkWidget *widget, ChmSee *chmsee)
{
  chmsee_ihtml_decrease_size(get_active_html(chmsee));
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

        glade = glade_xml_new(get_resource_path(GLADE_FILE), "about_dialog", NULL);
        dialog = glade_xml_get_widget(glade, "about_dialog");

        g_signal_connect(G_OBJECT (dialog),
                         "response",
                         G_CALLBACK (about_response_cb),
                         NULL);

        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG (dialog), PACKAGE_VERSION);

	g_object_unref(glade);
}

static void
hpanes_toggled_cb(GtkToggleToolButton *widget, ChmSee *self)
{
        gboolean state;
        g_object_get(widget, "active", &state, NULL);
        set_sidepane_state(self, state);
}

static void
on_open_new_tab(GtkWidget *widget, ChmSee *chmsee)
{
        ChmseeIhtml *html;
        const gchar *location;

        g_message("Open new tab");

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        location = chmsee_ihtml_get_location(html);

        if (location != NULL) {
          new_tab(chmsee, location);
        }
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
        g_debug("On context open new tab: %s", context_menu_link);

        if (context_menu_link != NULL)
                new_tab(chmsee, context_menu_link);
}

static void
on_context_copy_link(GtkWidget *widget, ChmSee *chmsee)
{
        g_debug("On context copy link: %s", context_menu_link);

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
    close_current_book(chmsee);
  }

  save_chmsee_config(chmsee);

  g_free(chmsee->home);
  g_free(chmsee->cache_dir);
  g_free(chmsee->last_dir);
  g_free(context_menu_link);

  if(get_active_html(chmsee)) {
    chmsee_ihtml_shutdown(get_active_html(chmsee));
  }

  gtk_main_quit();
  exit(0);

  g_message("chmsee quit");
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
populate_window(ChmSee *self)
{
        GladeXML *glade;

        glade = glade_xml_new(get_resource_path(GLADE_FILE), "main_vbox", NULL);

        if (glade == NULL) {
                g_error("Cannot find glade file!");
                exit(1);
        }

        g_object_set_data(G_OBJECT (self), "glade", glade);

        GtkWidget *main_vbox;
        main_vbox = get_widget(self, "main_vbox");
        gtk_container_add(GTK_CONTAINER (self), main_vbox);

        GtkAccelGroup *accel_group;

        accel_group = g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
        gtk_window_add_accel_group(GTK_WINDOW (self), accel_group);

        /* menu item */
        GtkWidget *menu_item;

        menu_item = get_widget(self, "menu_open");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_open),
                         self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_o,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(self, "menu_new_tab");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_open_new_tab),
                         self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_t,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(self, "menu_close_tab");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_close_current_tab),
                         self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_w,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(self, "menu_setup");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_setup),
                         self);

        menu_item = get_widget(self, "menu_copy");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_copy),
                         self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_c,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(self, "menu_quit");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (destroy_cb),
                         self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_q,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(self, "menu_home");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_home),
                         self);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(self, "menu_back");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_back),
                         self);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(self, "menu_forward");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_forward),
                         self);
        gtk_widget_set_sensitive(menu_item, FALSE);

        menu_item = get_widget(self, "menu_fullscreen");
        g_signal_connect_swapped(G_OBJECT(menu_item),
        		"toggled",
        		G_CALLBACK(on_fullscreen_toggled),
        		self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_F11,
                                   0,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(self, "menu_sidepane");
        g_signal_connect_swapped(G_OBJECT(menu_item),
        		"toggled",
        		G_CALLBACK(on_sidepane_toggled),
        		self);
        gtk_widget_add_accelerator(menu_item,
                                   "activate",
                                   accel_group,
                                   GDK_F9,
                                   0,
                                   GTK_ACCEL_VISIBLE);

        menu_item = get_widget(self, "menu_about");
        g_signal_connect(G_OBJECT (menu_item),
                         "activate",
                         G_CALLBACK (on_about),
                         self);

        /* toolbar buttons */
        GtkWidget *toolbar_button;
        GtkWidget *icon_widget;

        toolbar_button = get_widget(self, "toolbar_open");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_open),
                         self);

        toolbar_button = get_widget(self, "toolbar_setup");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_setup),
                         self);

        toolbar_button = get_widget(self, "toolbar_about");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_about),
                         NULL);

        toolbar_button = get_widget(self, "toolbar_hpanes");
        icon_widget = gtk_image_new_from_file(get_resource_path("show-pane.png"));
        gtk_widget_show(icon_widget);
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON (toolbar_button), icon_widget);
        g_object_set(toolbar_button, "active", FALSE, NULL);
        gtk_widget_set_sensitive(toolbar_button, FALSE);
        g_signal_connect(G_OBJECT (toolbar_button),
                         "toggled",
                         G_CALLBACK (hpanes_toggled_cb),
                         self);

        toolbar_button = get_widget(self, "toolbar_back");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_back),
                         self);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_Left,
                                   GDK_MOD1_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(self, "toolbar_forward");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_forward),
                         self);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_Right,
                                   GDK_MOD1_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(self, "toolbar_home");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_home),
                         self);

        toolbar_button = get_widget(self, "toolbar_zoom_in");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_in),
                         self);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_plus,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);


        toolbar_button = get_widget(self, "toolbar_zoom_reset");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_reset),
                         self);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_0,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        toolbar_button = get_widget(self, "toolbar_zoom_out");
        g_signal_connect(G_OBJECT (toolbar_button),
                         "clicked",
                         G_CALLBACK (on_zoom_out),
                         self);
        gtk_widget_add_accelerator(toolbar_button,
                                   "clicked",
                                   accel_group,
                                   GDK_minus,
                                   GDK_CONTROL_MASK,
                                   GTK_ACCEL_VISIBLE);

        GtkWidget *control_vbox;

        control_vbox = get_widget(self, "control_vbox");
        gtk_widget_hide(control_vbox);

        /* status bar */
        self->statusbar = glade_xml_get_widget(glade, "statusbar");
        self->scid_default = gtk_statusbar_get_context_id(GTK_STATUSBAR (self->statusbar),
                                                            "default");
        update_status_bar(self, _("Ready!"));
}

static void
display_book(ChmSee* self, ChmseeIchmfile *book)
{
	GNode *link_tree;
	GList *bookmarks_list;

	GtkWidget *booktree_sw;
	GtkWidget *control_vbox;

	g_message("display book");

	/* Close currently opened book */
	if (self->book)
		close_current_book(self);

	self->book = book;

	control_vbox = get_widget(self, "control_vbox");

	/* Book contents TreeView widget */
	self->control_notebook = gtk_notebook_new();

	gtk_box_pack_start(GTK_BOX (control_vbox),
			GTK_WIDGET (self->control_notebook),
			TRUE,
			TRUE,
			2);
	g_signal_connect(G_OBJECT (self->control_notebook),
			"switch-page",
			G_CALLBACK (control_switch_page_cb),
			self);

	/* TOC */
	if (chmsee_ichmfile_get_link_tree(self->book) != NULL) {
		link_tree = chmsee_ichmfile_get_link_tree(self->book);

		booktree_sw = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (booktree_sw),
				GTK_POLICY_NEVER,
				GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (booktree_sw),
				GTK_SHADOW_IN);
		gtk_container_set_border_width(GTK_CONTAINER (booktree_sw), 2);

		self->booktree = GTK_WIDGET (booktree_new(link_tree));

		gtk_container_add(GTK_CONTAINER (booktree_sw), self->booktree);
		gtk_notebook_append_page(GTK_NOTEBOOK (self->control_notebook),
				booktree_sw,
				gtk_label_new(_("Topics")));

		g_signal_connect(G_OBJECT (self->booktree),
				"link-selected",
				G_CALLBACK (booktree_link_selected_cb),
				self);

		g_message("chmsee has toc");
		self->has_toc = TRUE;
	}

	/* Bookmarks */
	bookmarks_list = chmsee_ichmfile_get_bookmarks_list(self->book);
	self->bookmark_tree = GTK_WIDGET (ui_bookmarks_new(bookmarks_list));

	gtk_notebook_append_page(GTK_NOTEBOOK (self->control_notebook),
			self->bookmark_tree,
			gtk_label_new (_("Bookmarks")));

	g_signal_connect(G_OBJECT (self->bookmark_tree),
			"link-selected",
			G_CALLBACK (bookmarks_link_selected_cb),
			self);

	GtkWidget *hpaned;

	hpaned = get_widget(self, "hpaned1");

	/* HTML tabs notebook */
	self->html_notebook = gtk_notebook_new();
	gtk_paned_add2 (GTK_PANED (hpaned), self->html_notebook);

	g_signal_connect(G_OBJECT (self->html_notebook),
			"switch-page",
			G_CALLBACK (html_switch_page_cb),
			self);

	gtk_widget_show_all(hpaned);
	new_tab(self, NULL);

	gtk_notebook_set_current_page(GTK_NOTEBOOK (self->control_notebook),
			g_list_length(bookmarks_list) && self->has_toc ? 1 : 0);

	/* Toolbar buttons state */
	GtkWidget *toolbar_button;

	toolbar_button = get_widget(self, "toolbar_hpanes");
	gtk_widget_set_sensitive(toolbar_button, TRUE);
	g_object_set(toolbar_button, "active", TRUE, NULL);

	toolbar_button = get_widget(self, "toolbar_zoom_in");
	gtk_widget_set_sensitive(toolbar_button, TRUE);
	toolbar_button = get_widget(self, "toolbar_zoom_reset");
	gtk_widget_set_sensitive(toolbar_button, TRUE);
	toolbar_button = get_widget(self, "toolbar_zoom_out");
	gtk_widget_set_sensitive(toolbar_button, TRUE);

	if (chmsee_ichmfile_get_home(self->book)) {
		GtkWidget *menu_item;

		open_homepage(self);

		menu_item = get_widget(self, "menu_new_tab");
		gtk_widget_set_sensitive(menu_item, TRUE);

		menu_item = get_widget(self, "menu_close_tab");
		gtk_widget_set_sensitive(menu_item, TRUE);

		menu_item = get_widget(self, "menu_home");
		gtk_widget_set_sensitive(menu_item, TRUE);

		gtk_widget_set_sensitive(get_widget(self, "menu_sidepane"), TRUE);

		toolbar_button = get_widget(self, "toolbar_home");
		gtk_widget_set_sensitive(toolbar_button, TRUE);
	}

	/* Window title */
	gchar *window_title;

	if (chmsee_ichmfile_get_title(self->book) != NULL
			&& g_ascii_strcasecmp(chmsee_ichmfile_get_title(self->book), "(null)") != 0 ) {
		window_title = g_strdup_printf("%s - ChmSee", chmsee_ichmfile_get_title(self->book));
	} else {
		window_title = g_strdup("ChmSee");
	}

	gtk_window_set_title(GTK_WINDOW (self), window_title);
	g_free(window_title);

	chmsee_ihtml_set_variable_font(get_active_html(self),
			chmsee_ichmfile_get_variable_font(self->book));
	chmsee_ihtml_set_fixed_font(get_active_html(self),
			chmsee_ichmfile_get_fixed_font(self->book));

	if(self->has_toc) {
		gtk_widget_grab_focus(self->booktree);
	}
}

static void
close_current_book(ChmSee *chmsee)
{
  gchar* bookmark_fname = g_build_filename(chmsee_ichmfile_get_dir(chmsee->book), CHMSEE_BOOKMARK_FILE, NULL);
  bookmarks_save(ui_bookmarks_get_list(UIBOOKMARKS (chmsee->bookmark_tree)), bookmark_fname);
  g_free(bookmark_fname);
  g_object_unref(chmsee->book);
  gtk_widget_destroy(GTK_WIDGET (chmsee->control_notebook));
  gtk_widget_destroy(GTK_WIDGET (chmsee->html_notebook));

  chmsee->book = NULL;
  chmsee->control_notebook = NULL;
  chmsee->html_notebook = NULL;
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
        ChmseeIhtml  *html;
        GtkWidget    *frame;
        GtkWidget    *view;
        GtkWidget    *tab_content;
        gint          num;

        g_debug("new_tab : %s", location);

        /* Ignore external link */
        if (location != NULL && !g_str_has_prefix(location, "file://"))
                return;

        html = chmsee_html_new();

        view = chmsee_ihtml_get_widget(html);
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
                chmsee_ihtml_open_uri(html, location);

                if (chmsee->has_toc)
                        booktree_select_uri(BOOKTREE (chmsee->booktree), location);
        } else {
                chmsee_ihtml_clear(html);
        }

        gtk_notebook_set_current_page(GTK_NOTEBOOK (chmsee->html_notebook), num);
}

static void
open_homepage(ChmSee *chmsee)
{
        ChmseeIhtml *html;

        html = get_active_html(chmsee);

        g_signal_handlers_block_by_func(html, html_open_uri_cb, chmsee);

        chmsee_ihtml_open_uri(html, g_build_filename(chmsee_ichmfile_get_dir(chmsee->book),
                                             chmsee_ichmfile_get_home(chmsee->book), NULL));

        g_signal_handlers_unblock_by_func(html, html_open_uri_cb, chmsee);

        if (chmsee->has_toc) {
          booktree_select_uri(BOOKTREE (chmsee->booktree),
                              chmsee_ichmfile_get_home(chmsee->book));
        }

        check_history(chmsee, html);
}

static void
reload_current_page(ChmSee *chmsee)
{
        ChmseeIhtml*html;
        const gchar *location;

        g_message("Reload current page");

        g_return_if_fail(GTK_IS_NOTEBOOK (chmsee->html_notebook));

        html = get_active_html(chmsee);
        location = chmsee_ihtml_get_location(html);

        if (location != NULL) {
          chmsee_ihtml_open_uri(html, location);
        }
}

static ChmseeIhtml *
get_active_html(ChmSee *chmsee)
{
        GtkWidget *frame;
        gint page_num;

        if(!chmsee->html_notebook) {
          return NULL;
        }
        page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK (chmsee->html_notebook));

        if (page_num == -1)
                return NULL;

        frame = gtk_notebook_get_nth_page(GTK_NOTEBOOK (chmsee->html_notebook), page_num);

        return g_object_get_data(G_OBJECT (frame), "html");
}

static void
check_history(ChmSee *chmsee, ChmseeIhtml *html)
{
        GtkWidget *menu_item, *toolbar_button;
        gboolean back_state, forward_state;

        back_state = chmsee_ihtml_can_go_back(html);
        forward_state = chmsee_ihtml_can_go_forward(html);

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
update_tab_title(ChmSee *chmsee, ChmseeIhtml *html)
{
  const gchar* html_title;
  const gchar* tab_title;
  const gchar* book_title;

        html_title = chmsee_ihtml_get_title(html);

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
}

static void
tab_set_title(ChmSee *chmsee, ChmseeIhtml *html, const gchar *title)
{
        GtkWidget *view;
        GtkWidget *page;
        GtkWidget *widget, *label;
        gint num_pages, i;

        view = chmsee_ihtml_get_widget(html);

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
chmsee_new(const gchar* filename)
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

        /* Init gecko */
        chmsee_html_init_system();
        chmsee_html_set_default_lang(chmsee->lang);

        populate_window(chmsee);
        load_chmsee_config(chmsee);
        if (chmsee->pos_x >= 0 && chmsee->pos_y >= 0)
                gtk_window_move(GTK_WINDOW (chmsee), chmsee->pos_x, chmsee->pos_y);

        if (chmsee->width > 0 && chmsee->height > 0)
                gtk_window_resize(GTK_WINDOW (chmsee), chmsee->width, chmsee->height);
        else
                gtk_window_resize(GTK_WINDOW (chmsee), 800, 600);

        gtk_window_set_title(GTK_WINDOW (chmsee), "ChmSee");
        gtk_window_set_icon_from_file(GTK_WINDOW (chmsee), get_resource_path("chmsee-icon.png"), NULL);

        if(filename != NULL) {
        	chmsee_open_file(chmsee, filename);
        }

        return chmsee;
}

void
chmsee_open_file(ChmSee *chmsee, const gchar *filename)
{
        ChmseeIchmfile* book;

        g_return_if_fail(IS_CHMSEE (chmsee));

        /* Extract chm and get file infomation */
        book = chmsee_chmfile_new(filename);

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

void
chmsee_drag_data_received (GtkWidget          *widget,
                           GdkDragContext     *context,
                           gint                x,
                           gint                y,
                           GtkSelectionData   *selection_data,
                           guint               info,
                           guint               time)
{
  gchar  **uris;
  gint     i = 0;

  uris = gtk_selection_data_get_uris (selection_data);
  if (!uris) {
    gtk_drag_finish (context, FALSE, FALSE, time);
    return;
  }

  for (i = 0; uris[i]; i++) {
    gchar* uri = uris[i];
    if(g_str_has_prefix(uri, "file://")
       && (g_str_has_suffix(uri, ".chm")
           || g_str_has_suffix(uri, ".CHM"))) {
      chmsee_open_uri(CHMSEE(widget), uri);
      break;
    }
  }

  gtk_drag_finish (context, TRUE, FALSE, time);

  g_strfreev (uris);
}

void chmsee_open_uri(ChmSee *chmsee, const gchar *uri) {
  if(!g_str_has_prefix(uri, "file://")) {
    return;
  }

  gchar* fname = g_uri_unescape_string(uri+7, NULL);
  chmsee_open_file(chmsee, fname);
  g_free(fname);
}

int chmsee_get_hpaned_position(ChmSee* self) {
	gint position;
	g_object_get(G_OBJECT(get_widget(self, "hpaned1")),
			"position", &position,
			NULL
			);
	return position;
}

void chmsee_set_hpaned_position(ChmSee* self, int hpaned_position) {
	self->hpaned_position = hpaned_position;
	/*
	g_object_set(G_OBJECT(get_widget(self, "hpaned1")),
			"position", hpaned_position,
			NULL
			);
			*/
}

void on_fullscreen_toggled(ChmSee* self, GtkWidget* menu) {
	g_return_if_fail(IS_CHMSEE(self));
	gboolean active;
	g_object_get(G_OBJECT(menu),
			"active", &active,
			NULL);
	if(active) {
		gtk_window_fullscreen(GTK_WINDOW(self));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(self));
	}
}

void on_sidepane_toggled(ChmSee* self, GtkWidget* menu) {
	g_return_if_fail(IS_CHMSEE(self));
	gboolean active;
	g_object_get(G_OBJECT(menu),
			"active", &active,
			NULL);
	if(active) {
		show_sidepane(self);
	} else {
		hide_sidepane(self);
	}
}

void set_sidepane_state(ChmSee* self, gboolean state) {
	GtkWidget* icon_widget;

	if(state) {
		gtk_widget_show(get_widget(self, "control_vbox"));
	} else {
		gtk_widget_hide(get_widget(self, "control_vbox"));
	}

	gboolean menu_state = gtk_check_menu_item_get_active(
			GTK_CHECK_MENU_ITEM(get_widget(self, "menu_sidepane")));
	if(menu_state != state) {
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(get_widget(self, "menu_sidepane")), state);
	}

	gboolean toolbar_state = gtk_toggle_tool_button_get_active(
			GTK_TOGGLE_TOOL_BUTTON(get_widget(self, "toolbar_hpanes")));
	if(toolbar_state != state) {
		gtk_toggle_tool_button_set_active(
				GTK_TOGGLE_TOOL_BUTTON(get_widget(self, "toolbar_hpanes")), state);
	}

    if (state) {
            icon_widget = gtk_image_new_from_file(get_resource_path("hide-pane.png"));
    } else {
            icon_widget = gtk_image_new_from_file(get_resource_path("show-pane.png"));
    }
    gtk_widget_show(icon_widget);
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON (get_widget(self, "toolbar_hpanes")), icon_widget);
};

void show_sidepane(ChmSee* self) {
	set_sidepane_state(self, TRUE);
}

void hide_sidepane(ChmSee* self) {
	set_sidepane_state(self, FALSE);
}


void on_map(ChmSee* self) {
	if(self->hpaned_position >= 0) {
	g_object_set(G_OBJECT(get_widget(self, "hpaned1")),
			"position", self->hpaned_position,
			NULL
			);
	}
}


static void on_fullscreen(ChmSee* self) {
	gtk_widget_hide(get_widget(self, "handlebox_menu"));
	gtk_widget_hide(get_widget(self, "handlebox_toolbar"));
	gtk_widget_hide(get_widget(self, "statusbar"));
	self->fullscreen = TRUE;
}

static void on_unfullscreen(ChmSee* self) {
	gtk_widget_show(get_widget(self, "handlebox_menu"));
	gtk_widget_show(get_widget(self, "handlebox_toolbar"));
	gtk_widget_show(get_widget(self, "statusbar"));
	self->fullscreen = FALSE;
}

gboolean on_window_state_event(ChmSee* self, GdkEventWindowState* event) {
	g_return_val_if_fail(IS_CHMSEE(self), FALSE);
	g_return_val_if_fail(event->type == GDK_WINDOW_STATE, FALSE);

	if(!(event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
		return FALSE;
	}

	if(event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) {
		on_fullscreen(self);
	} else {
		on_unfullscreen(self);
	}

	return FALSE;
}
