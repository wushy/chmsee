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

#ifndef __BOOKTREE_H__
#define __BOOKTREE_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "models/link.h"
#include "models/hhc.h"

#define TYPE_BOOKTREE \
        (booktree_get_type())
#define BOOKTREE(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BOOKTREE, BookTree))
#define BOOKTREE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_BOOKTREE, BookTreeClass))
#define IS_BOOKTREE(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BOOKTREE))
#define IS_BOOKTREE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((o), TYPE_BOOKTREE))

typedef struct {
        GdkPixbuf *pixbuf_opened;
        GdkPixbuf *pixbuf_closed;
        GdkPixbuf *pixbuf_doc;
} BookTreePixbufs;

typedef struct {
        const gchar *uri;
        gboolean     found;
        GtkTreeIter  iter;
        GtkTreePath *path;
} FindURIData;

typedef struct _BookTree       BookTree;
typedef struct _BookTreeClass  BookTreeClass;

struct _BookTree {
        GtkTreeView      parent;

        GtkTreeStore    *store;

        BookTreePixbufs *pixbufs;
        Hhc             *link_tree;
};

struct _BookTreeClass {
        GtkTreeViewClass parent_class;

        /* Signals */
        void (*link_selected) (BookTree *booktree, Link *link);
};

GType booktree_get_type(void);
GtkWidget *booktree_new(GNode *);

void booktree_select_uri(BookTree *, const gchar *);
const gchar *booktree_get_selected_book_title(BookTree *);

#endif /* !__BOOKTREE_H__ */
