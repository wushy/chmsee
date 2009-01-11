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

#ifndef __UI_BOOKMARKS_H__
#define __UI_BOOKMARKS_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "models/link.h"

#define TYPE_UIBOOKMARKS \
        (ui_bookmarks_get_type())
#define UIBOOKMARKS(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_UIBOOKMARKS, UiBookmarks))
#define UIBOOKMARKS_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_UIBOOKMARKS, UiBookmarksClass))
#define IS_UIBOOKMARKS(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_UIBOOKMARKS))
#define IS_SEARCH_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_UIBOOKMARKS))

typedef struct _UiBookmarks       UiBookmarks;
typedef struct _UiBookmarksClass  UiBookmarksClass;

struct _UiBookmarks {
        GtkVBox        parent;
        
        GtkWidget     *list;
        GtkListStore  *store;

        GtkWidget     *entry;
        GtkWidget     *add_button;
        GtkWidget     *remove_button;

        GList         *links;
        gchar         *current_uri;
};

struct _UiBookmarksClass {
        GtkVBoxClass   parent_class;

        /* Signals */
        void (*link_selected) (UiBookmarks *bookmarks, Link *link);
};

GType ui_bookmarks_get_type(void);
GtkWidget *ui_bookmarks_new(GList *);

void ui_bookmarks_set_current_link(UiBookmarks *, const gchar *, const gchar *);
void ui_bookmarks_grab_focus(UiBookmarks *);
GList *ui_bookmarks_get_list(UiBookmarks *);

#endif /* !__UI_BOOKMARKS_H__ */
