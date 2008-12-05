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

#ifndef __BOOKMARKS_H__
#define __BOOKMARKS_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "link.h"

#define TYPE_BOOKMARKS \
        (bookmarks_get_type())
#define BOOKMARKS(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_BOOKMARKS, Bookmarks))
#define BOOKMARKS_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_BOOKMARKS, BookmarksClass))
#define IS_BOOKMARKS(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_BOOKMARKS))
#define IS_SEARCH_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_BOOKMARKS))

typedef struct _Bookmarks       Bookmarks;
typedef struct _BookmarksClass  BookmarksClass;

struct _Bookmarks {
        GtkVBox        parent;
        
        GtkWidget     *list;
        GtkListStore  *store;

        GtkWidget     *entry;
        GtkWidget     *add_button;
        GtkWidget     *remove_button;

        GList         *links;
        gchar         *current_uri;
};

struct _BookmarksClass {
        GtkVBoxClass   parent_class;

        /* Signals */
        void (*link_selected) (Bookmarks *bookmarks, Link *link);
};

GType bookmarks_get_type(void);
GtkWidget *bookmarks_new(GList *);

void bookmarks_set_current_link(Bookmarks *, const gchar *, const gchar *);
void bookmarks_grab_focus(Bookmarks *);
GList *bookmarks_get_list(Bookmarks *);

#endif /* !__BOOKMARKS_H__ */
