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
 
#include <stdio.h>
#include <string.h>

#include "bookmarks.h"
#include "utils.h"

static void bookmarks_init(Bookmarks *);
static void bookmarks_class_init(BookmarksClass *);
static void bookmarks_finalize(GObject *);

static void row_activated_cb(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, Bookmarks *);
static void selection_changed_cb(GtkTreeSelection *, Bookmarks *);
static void entry_changed_cb(GtkEntry *, Bookmarks *);
static void on_bookmark_add(GtkWidget *, Bookmarks *);
static void on_bookmark_remove(GtkWidget *, Bookmarks *);

static void update_bookmarks_treeview(Bookmarks *, gchar *);
static gint link_uri_compare(gconstpointer a, gconstpointer b);

/* Signals */
enum {
        LINK_SELECTED,
        LAST_SIGNAL
};

/* TreeView Model */
enum {
        COL_TITLE,
        COL_URI,
        N_COLUMNS
};

static GtkVBox *parent_class;
static gint signals[LAST_SIGNAL] = { 0 };

GType
bookmarks_get_type(void)
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info =
                        {
                                sizeof(BookmarksClass),
                                NULL,
                                NULL,
                                (GClassInitFunc)bookmarks_class_init,
                                NULL,
                                NULL,
                                sizeof(Bookmarks),
                                0,
                                (GInstanceInitFunc)bookmarks_init,
                        };
                
                type = g_type_register_static(GTK_TYPE_VBOX,
                                              "Bookmarks", 
                                              &info, 0);
        }
        
        return type;
}

static void
bookmarks_class_init(BookmarksClass *klass)
{
        GObjectClass *object_class;
        
        object_class = (GObjectClass *)klass;
        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = bookmarks_finalize;
        
        signals[LINK_SELECTED] =
                g_signal_new("link_selected",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET (BookmarksClass, link_selected),
                             NULL, 
                             NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1, 
                             G_TYPE_POINTER);
}

static void
bookmarks_init(Bookmarks *bookmarks)
{
        bookmarks->current_uri = NULL;
        bookmarks->links = NULL;

        bookmarks->list = gtk_tree_view_new();
        bookmarks->store = gtk_list_store_new(N_COLUMNS, 
                                              G_TYPE_STRING, 
                                              G_TYPE_STRING);

        gtk_tree_view_set_model(GTK_TREE_VIEW (bookmarks->list),
                                GTK_TREE_MODEL (bookmarks->store));

        gtk_tree_view_set_enable_search(GTK_TREE_VIEW (bookmarks->list), FALSE);

        gtk_box_set_spacing(GTK_BOX (bookmarks), 2);
}

static void
bookmarks_finalize(GObject *object)
{
        if (G_OBJECT_CLASS (parent_class)->finalize)
                G_OBJECT_CLASS (parent_class)->finalize(object);
}

/* callbacks */

static void
row_activated_cb(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, Bookmarks *bookmarks)
{
        GtkTreeModel *model = GTK_TREE_MODEL (bookmarks->store);
        GtkTreeIter  iter;
        gchar *title, *uri;
        GList *find_link;
        Link *link;

        gtk_tree_model_get_iter(model, &iter, path);
        gtk_tree_model_get(model, 
                           &iter, 
                           COL_TITLE, &title, 
                           COL_URI, &uri, 
                           -1);

        find_link = g_list_find_custom(bookmarks->links, uri, link_uri_compare);

        if (find_link) {
                link = LINK (find_link->data);

                d(g_debug("Emiting signal with link to: %s title=%s\n", link->uri, title));

                g_signal_emit(bookmarks, signals[LINK_SELECTED], 0, link);
        }
}

static void
selection_changed_cb(GtkTreeSelection *selection, Bookmarks *bookmarks)
{
        d(g_message("bookmark selection_changed"));
}

static void
entry_changed_cb(GtkEntry *entry, Bookmarks *bookmarks) 
{
        const gchar *name;
        gint length;

        name = gtk_entry_get_text(entry);
        length = strlen(name);
        
        if (length >= 2)
                gtk_widget_set_sensitive(bookmarks->add_button, TRUE);
        else
                gtk_widget_set_sensitive(bookmarks->add_button, FALSE);
}

static void
on_bookmark_add(GtkWidget *widget, Bookmarks *bookmarks)
{
        gchar *name;
        GList *find_link;
        Link *link;

        d(g_message("on_bookmark_add"));

        name = g_strdup(gtk_entry_get_text(GTK_ENTRY (bookmarks->entry)));

        find_link = g_list_find_custom(bookmarks->links, bookmarks->current_uri, link_uri_compare);

        if (find_link == NULL) { /* new bookmark item */
                link = link_new(LINK_TYPE_PAGE, name, bookmarks->current_uri);
                bookmarks->links = g_list_append(bookmarks->links, link);
        } else {
                link = LINK (find_link->data);

                /* Update bookmark item name */
                if (ncase_compare_utf8_string(link->name, name) != 0) { 
                        g_free(link->name);
                        link->name = g_strdup(name);
                }
        }
        
        g_free(name);
        update_bookmarks_treeview(bookmarks, link->uri);
}

static void
on_bookmark_remove(GtkWidget *widget, Bookmarks *bookmarks)
{
        GtkTreeSelection *selection;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (bookmarks->list));

        if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
                GList *list;
                gchar *title, *uri;

                gtk_tree_model_get(GTK_TREE_MODEL (bookmarks->store),
                                   &iter,
                                   COL_TITLE, &title,
                                   COL_URI, &uri,
                                   -1);

                list = g_list_find_custom(bookmarks->links, uri, link_uri_compare);

                Link *link = (Link *)list->data;
                bookmarks->links = g_list_remove(bookmarks->links, link);
                link_free(link);
                
                update_bookmarks_treeview(bookmarks, NULL);
        }
}

/* internal functions */

static void
update_bookmarks_treeview(Bookmarks *bookmarks, gchar *uri)
{
        GtkTreeSelection *selection;
        GtkTreeIter iter;
        GList *l;

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (bookmarks->list));
        gtk_list_store_clear(bookmarks->store);
        
        for (l = bookmarks->links; l; l = l->next) {
                Link *link = l->data;

                gtk_list_store_append(bookmarks->store, &iter);
                gtk_list_store_set(bookmarks->store, &iter,
                                   COL_TITLE, link->name,
                                   COL_URI, link->uri,
                                   -1);

                if (uri != NULL && ncase_compare_utf8_string(uri, link->uri) == 0)
                        gtk_tree_selection_select_iter(selection, &iter);
        }

        if (uri == NULL || strlen(uri) == 0)
                gtk_tree_selection_select_path(selection, gtk_tree_path_new_first());
}

static gint
link_uri_compare(gconstpointer a, gconstpointer b)
{
        return ncase_compare_utf8_string(((Link *)a)->uri, (char *)b);
}

/* external functions */

GtkWidget *
bookmarks_new(GList *links)
{
        Bookmarks        *bookmarks;

        GtkTreeSelection *selection;
        GtkWidget        *list_sw;
        GtkWidget        *frame;
        GtkWidget        *hbox;

        GtkCellRenderer  *cell;
                
        bookmarks = g_object_new(TYPE_BOOKMARKS, NULL);

        gtk_container_set_border_width(GTK_CONTAINER (bookmarks), 2);
        
        bookmarks->links = links;
        
        /* bookmarks list */
        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_IN);
        
        list_sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (list_sw),
                                       GTK_POLICY_NEVER, 
                                       GTK_POLICY_AUTOMATIC);

        gtk_container_add(GTK_CONTAINER (frame), list_sw);

        cell = gtk_cell_renderer_text_new();
        g_object_set(cell,
                     "ellipsize", PANGO_ELLIPSIZE_END,
                     NULL);
        
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (bookmarks->list), 
                                                    -1,
                                                    _("Bookmark"), cell,
                                                    "text", 0,
                                                    NULL);

        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (bookmarks->list), FALSE);
        gtk_tree_view_set_search_column(GTK_TREE_VIEW (bookmarks->list), FALSE);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (bookmarks->list));

        g_signal_connect(selection, 
                         "changed",
                         G_CALLBACK (selection_changed_cb),
                         bookmarks);
        
        g_signal_connect(bookmarks->list, 
                         "row_activated",
                         G_CALLBACK (row_activated_cb),
                         bookmarks);
        
        gtk_container_add(GTK_CONTAINER (list_sw), bookmarks->list);

        gtk_box_pack_start(GTK_BOX (bookmarks), frame, TRUE, TRUE, 0);

        /* bookmark title */
        bookmarks->entry = gtk_entry_new();

        g_signal_connect(bookmarks->entry, 
                         "changed", 
                         G_CALLBACK (entry_changed_cb),
                         bookmarks);

        gtk_box_pack_start(GTK_BOX (bookmarks), bookmarks->entry, FALSE, FALSE, 2);

        /* bookmark add and remove buttons */
        hbox = gtk_hbox_new(FALSE, 0);

        bookmarks->add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);

        g_signal_connect(G_OBJECT (bookmarks->add_button), 
                         "clicked",
                         G_CALLBACK (on_bookmark_add),
                         bookmarks);

        bookmarks->remove_button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
        g_signal_connect(G_OBJECT (bookmarks->remove_button), 
                         "clicked",
                         G_CALLBACK (on_bookmark_remove),
                         bookmarks);

        gtk_box_pack_end(GTK_BOX (hbox), bookmarks->add_button, TRUE, TRUE, 0);
        gtk_box_pack_end(GTK_BOX (hbox), bookmarks->remove_button, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX (bookmarks), hbox, FALSE, FALSE, 2);

        update_bookmarks_treeview(bookmarks, NULL);
        gtk_widget_show_all(GTK_WIDGET (bookmarks));

        return GTK_WIDGET(bookmarks);
}

void
bookmarks_set_current_link(Bookmarks *bookmarks, const gchar *name, const gchar *uri)
{
        g_return_if_fail(IS_BOOKMARKS (bookmarks));

        d(g_message("Call bookmarks_set_current_link"));

        d(g_debug("set entry text: %s", name));
        gtk_entry_set_text(GTK_ENTRY (bookmarks->entry), name);
        
        gtk_editable_set_position(GTK_EDITABLE (bookmarks->entry), -1);
        gtk_editable_select_region(GTK_EDITABLE (bookmarks->entry), -1, -1);

        g_free(bookmarks->current_uri);

        bookmarks->current_uri = g_strdup(uri);
}

void
bookmarks_grab_focus(Bookmarks *bookmarks)
{
        g_return_if_fail(IS_BOOKMARKS (bookmarks));

        gtk_widget_grab_focus(bookmarks->entry);
}

GList *
bookmarks_get_list(Bookmarks *bookmarks)
{
        return bookmarks->links;
}
