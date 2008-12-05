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

/***************************************************************************
 *   Copyright (C) 2003 by zhong                                           *
 *   zhongz@163.com                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __CHMSEE_H__
#define __CHMSEE_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib-object.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtk.h>

#include "chmfile.h"

typedef struct _ChmSee      ChmSee;
typedef struct _ChmSeeClass ChmSeeClass;

#define TYPE_CHMSEE \
        (chmsee_get_type ())
#define CHMSEE(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_CHMSEE, ChmSee))
#define CHMSEE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_CHMSEE, ChmSeeClass))
#define IS_CHMSEE(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_CHMSEE))
#define IS_CHMSEE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_CHMSEE))
#define CHMSEE_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_CHMSEE, ChmSeeClass))


struct _ChmSee {
        GtkWindow        parent;

        GtkWidget       *control_notebook;
        GtkWidget       *html_notebook;

        GtkWidget       *booktree;        
        GtkWidget       *bookmark_tree;
        GtkWidget       *index_tree;
        
        GtkWidget       *statusbar;
        guint            scid_default;

        gboolean         has_toc;
        gboolean         has_index;
        gint             pos_x;
        gint             pos_y;
        gint             width;
        gint             height;

        gint             lang;

        ChmFile         *book;

        gchar           *home;
        gchar           *cache_dir;
        gchar           *last_dir;
};

struct _ChmSeeClass {
        GtkWindowClass   parent_class;
};

GType chmsee_get_type(void);
ChmSee * chmsee_new(void);
void chmsee_open_file(ChmSee *, const gchar *);

#endif /* !__CHMSEE_H__ */
