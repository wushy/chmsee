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

#ifndef __CHMFILE_H__
#define __CHMFILE_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktypeutils.h>

typedef struct _ChmFile       ChmFile;
typedef struct _ChmFileClass  ChmFileClass;

#define TYPE_CHMFILE \
        (chmfile_get_type ())
#define CHMFILE(o) \
        (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_CHMFILE, ChmFile))
#define CHMFILE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_CHMFILE, ChmFileClass))
#define IS_CHMFILE(o) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_CHMFILE))
#define IS_CHMFILE_CLASS(k) \
        (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_CHMFILE))
#define CHMFILE_GET_CLASS(o) \
        (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_CHMFILE, ChmFileClass))

struct _ChmFile
{
        GObject         parent;

        gchar          *filename;
        gchar          *dir;
        gchar          *home;
        gchar          *hhc;
        gchar          *hhk;
        gchar          *title;
        const gchar    *encoding;
        gchar          *variable_font;
        gchar          *fixed_font;

        GList          *bookmarks_list;
        GNode          *link_tree;
};

struct _ChmFileClass
{
	GObjectClass parent_class;
};

GtkType chmfile_get_type(void);
ChmFile *chmfile_new(const gchar *);

#endif /* !__CHMFILE_H__ */
