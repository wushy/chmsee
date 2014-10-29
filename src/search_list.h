/*
 *  Copyright (c) 2014 Xianguang Zhou <624146104@qq.com>
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

#ifndef __SEARCH_LIST_H__
#define __SEARCH_LIST_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SearchList      SearchList;
typedef struct _SearchListPrivate SearchListPrivate;
typedef struct _SearchListClass SearchListClass;

#define TYPE_SEARCHLIST \
        (searchlist_get_type ())
#define SEARCHLIST(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SEARCHLIST, SearchList))
#define SEARCHLIST_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_SEARCHLIST, SearchListClass))
#define IS_SEARCHLIST(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SEARCHLIST))
#define IS_SEARCHLIST_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SEARCHLIST))
#define SEARCHLIST_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SEARCHLIST, SearchListClass))

struct _SearchList {
		GtkVBox        parent;
        SearchListPrivate*  priv;
};

struct _SearchListClass {
		GtkVBoxClass   parent_class;
};

GType searchlist_get_type(void);
GtkWidget * searchlist_new(const gchar * book_dir, const gchar * home_dir);

G_END_DECLS

#endif /* !__SEARCH_LIST_H__ */
