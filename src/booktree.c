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

#include "booktree.h"
#include "utils.h"

static void booktree_class_init(BookTreeClass *);
static void booktree_init(BookTree *);
static void booktree_finalize(GObject *);

static void booktree_selection_changed_cb(GtkTreeSelection *, BookTree *);

static void booktree_create_pixbufs(BookTree *);
static void booktree_add_columns(BookTree *);
static void booktree_setup_selection(BookTree *);
static void booktree_populate_tree(BookTree *);
static void booktree_insert_node(BookTree *, GNode *, GtkTreeIter *);

/* Signals */
enum {
        LINK_SELECTED,
        LAST_SIGNAL
};

enum {
        COL_OPEN_PIXBUF,
        COL_CLOSED_PIXBUF,
        COL_TITLE,
        COL_LINK,
        N_COLUMNS
};

static GtkTreeViewClass *parent_class = NULL;
static gint              signals[LAST_SIGNAL] = { 0 };

GType
booktree_get_type (void)
{
        static GType type = 0;

        if (!type) {
                static const GTypeInfo info = {
                        sizeof (BookTreeClass),
                        NULL,
                        NULL,
                        (GClassInitFunc)booktree_class_init,
                        NULL,
                        NULL,
                        sizeof (BookTree),
                        0,
                        (GInstanceInitFunc)booktree_init,
                };

                type = g_type_register_static(GTK_TYPE_TREE_VIEW,
                                              "BookTree",
                                              &info, 0);
        }

        return type;
}

static void
booktree_class_init(BookTreeClass *klass)
{
        GObjectClass *object_class;

        object_class = (GObjectClass *)klass;
        parent_class = g_type_class_peek_parent(klass);

        object_class->finalize = booktree_finalize;

        signals[LINK_SELECTED] =
                g_signal_new ("link_selected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (BookTreeClass, link_selected),
                              NULL, 
                              NULL,
                              g_cclosure_marshal_VOID__POINTER,
                              G_TYPE_NONE,
                              1, 
                              G_TYPE_POINTER);
}

static void
booktree_init(BookTree *tree)
{
        tree->store = gtk_tree_store_new(N_COLUMNS,
                                         GDK_TYPE_PIXBUF,
                                         GDK_TYPE_PIXBUF,
                                         G_TYPE_STRING,
                                         G_TYPE_POINTER);
        gtk_tree_view_set_model(GTK_TREE_VIEW (tree),
                                GTK_TREE_MODEL (tree->store));
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (tree), FALSE);

        booktree_create_pixbufs(tree);
        booktree_add_columns(tree);
        booktree_setup_selection(tree);
}

static void
booktree_finalize(GObject *object)
{
        BookTree *tree;

        tree = BOOKTREE (object);

        g_object_unref(tree->store);

        g_object_unref(tree->pixbufs->pixbuf_opened);
        g_object_unref(tree->pixbufs->pixbuf_closed);
        g_object_unref(tree->pixbufs->pixbuf_doc);
        g_free(tree->pixbufs);

        if (G_OBJECT_CLASS (parent_class)->finalize)
                G_OBJECT_CLASS (parent_class)->finalize(object);
}

/* internal functions */

static void
booktree_create_pixbufs(BookTree *tree)
{
        BookTreePixbufs *pixbufs;

        pixbufs = g_new0(BookTreePixbufs, 1);

        pixbufs->pixbuf_closed = gdk_pixbuf_new_from_file(CHMSEE_DATA_DIR "/book-closed.png", NULL);
        pixbufs->pixbuf_opened = gdk_pixbuf_new_from_file(CHMSEE_DATA_DIR "/book-open.png", NULL);
        pixbufs->pixbuf_doc = gdk_pixbuf_new_from_file(CHMSEE_DATA_DIR "/helpdoc.png", NULL);

        tree->pixbufs = pixbufs;
}

static void
booktree_add_columns(BookTree *tree)
{
        GtkCellRenderer   *cell;
        GtkTreeViewColumn *column;

        column = gtk_tree_view_column_new();

        cell = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(
                column,
                cell,
                "pixbuf", COL_OPEN_PIXBUF,
                "pixbuf-expander-open", COL_OPEN_PIXBUF,
                "pixbuf-expander-closed", COL_CLOSED_PIXBUF,
                NULL);

        cell = gtk_cell_renderer_text_new();
        g_object_set(cell,
                     "ellipsize", PANGO_ELLIPSIZE_END,
                     NULL);
        gtk_tree_view_column_pack_start(column, cell, TRUE);
        gtk_tree_view_column_set_attributes(column, cell,
                                            "text", COL_TITLE,
                                            NULL);

        gtk_tree_view_append_column(GTK_TREE_VIEW (tree), column);
}

static void
booktree_setup_selection(BookTree *tree)
{
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);

        g_signal_connect(selection, 
                         "changed",
                         G_CALLBACK (booktree_selection_changed_cb),
                         tree);
}

static void
booktree_populate_tree(BookTree *tree)
{
        GNode *node;

        for (node = g_node_first_child(tree->link_tree);
             node;
             node = g_node_next_sibling(node))
                booktree_insert_node(tree, node, NULL);
}

static void
booktree_insert_node(BookTree *tree, GNode *node, GtkTreeIter *parent_iter)
{
        GtkTreeIter iter;
        Link *link;
        GNode *child;

        link = node->data;

        if (g_node_n_children(node))
                link_change_type(link, LINK_TYPE_BOOK);

        gtk_tree_store_append(tree->store, &iter, parent_iter);

/*         d(g_debug("insert node::name = %s", link->name)); */
/*         d(g_debug("insert node::uri = %s", link->uri)); */

        if (link->type == LINK_TYPE_BOOK)
                gtk_tree_store_set(tree->store, &iter,
                                   COL_OPEN_PIXBUF, tree->pixbufs->pixbuf_opened,
                                   COL_CLOSED_PIXBUF, tree->pixbufs->pixbuf_closed,
                                   COL_TITLE, link->name,
                                   COL_LINK, link,
                                   -1);
        else
                gtk_tree_store_set(tree->store, &iter,
                                   COL_OPEN_PIXBUF, tree->pixbufs->pixbuf_doc,
                                   COL_CLOSED_PIXBUF, tree->pixbufs->pixbuf_doc,
                                   COL_TITLE, link->name,
                                   COL_LINK, link,
                                   -1);

        for (child = g_node_first_child(node);
             child;
             child = g_node_next_sibling(child))
                booktree_insert_node(tree, child, &iter);
}

static gboolean
booktree_find_uri_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, FindURIData *data)
{
        Link *link;

        gtk_tree_model_get(model, iter, COL_LINK, &link, -1);

        if (g_str_has_suffix(data->uri, link->uri)) {
                d(g_debug("data->uri: %s", data->uri));
                d(g_debug("link->uri: %s", link->uri));

                data->found = TRUE;
                data->iter = *iter;
                data->path = gtk_tree_path_copy(path);
        }

        return data->found;
}

/* callbacks */

static void
booktree_selection_changed_cb(GtkTreeSelection *selection, BookTree *tree)
{
        GtkTreeIter iter;
        Link *link;

        if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
                gtk_tree_model_get(GTK_TREE_MODEL (tree->store),
                                   &iter, COL_LINK, &link, -1);

                d(g_debug("book tree emiting '%s'\n", link->uri));

                g_signal_emit(tree, signals[LINK_SELECTED], 0, link);
        }
}

/* external functions */

GtkWidget *
booktree_new(GNode *link_tree)
{
        BookTree *tree;

        tree = g_object_new(TYPE_BOOKTREE, NULL);

        tree->link_tree = link_tree;

        booktree_populate_tree(tree);

        return GTK_WIDGET (tree);
}

void
booktree_select_uri(BookTree *tree, const gchar *uri)
{
        GtkTreeSelection *selection;
        FindURIData data;
        gchar *real_uri;

        g_return_if_fail(IS_BOOKTREE (tree));

        real_uri = get_real_uri(uri);

        data.found = FALSE;
        data.uri = real_uri;

        gtk_tree_model_foreach(GTK_TREE_MODEL (tree->store),
                               (GtkTreeModelForeachFunc) booktree_find_uri_foreach,
                               &data);

        if (!data.found) {
                d(g_message("booktree select uri: cannot found data"));
                return;
        }

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree));

        g_signal_handlers_block_by_func(selection,
                                        booktree_selection_changed_cb,
                                        tree);

        gtk_tree_view_expand_to_path(GTK_TREE_VIEW (tree), data.path);
        gtk_tree_selection_select_iter(selection, &data.iter);
        gtk_tree_view_set_cursor(GTK_TREE_VIEW (tree), data.path, NULL, 0);

        g_signal_handlers_unblock_by_func(selection,
                                          booktree_selection_changed_cb,
                                          tree);

        gtk_tree_path_free(data.path);
        g_free(real_uri);
}

const gchar *
booktree_get_selected_book_title(BookTree *tree)
{
        GtkTreeSelection *selection;
        GtkTreeModel     *model;
        GtkTreeIter       iter;
        GtkTreePath      *path;
        Link             *link;

        g_return_val_if_fail(IS_BOOKTREE (tree), NULL);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree));

        if (!gtk_tree_selection_get_selected(selection, &model, &iter))
                return NULL;

        path = gtk_tree_model_get_path(model, &iter);

        /* Get the book node for this link. */
        while (gtk_tree_path_get_depth(path) > 1)
                gtk_tree_path_up(path);

        gtk_tree_model_get_iter(model, &iter, path);
        gtk_tree_path_free(path);

        gtk_tree_model_get(model, &iter,
                           COL_LINK, &link,
                           -1);

        return link->name;
}

